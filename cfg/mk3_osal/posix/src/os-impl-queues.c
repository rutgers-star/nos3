/* MK3 process-local OSAL queues. cFS tasks are pthreads in one process, so
 * they can exchange bounded messages without the kernel's POSIX mqueue API. */

#include "os-posix.h"
#include "bsp-impl.h"
#include "os-impl-queues.h"
#include "os-shared-queue.h"
#include "os-shared-idmap.h"

#include <stdint.h>

OS_impl_queue_internal_record_t OS_impl_queue_table[OS_MAX_QUEUES];

/* pthread_cond_wait reacquires the mutex before running cancellation cleanup. */
static void MK3_QueueWaitCleanup(void *argument)
{
    OS_impl_queue_internal_record_t *impl = argument;
    --impl->waiters;
    if (!impl->open && impl->waiters == 0)
    {
        pthread_cond_signal(&impl->no_waiters);
    }
    pthread_mutex_unlock(&impl->mutex);
}

int32 OS_Posix_QueueAPI_Impl_Init(void)
{
    size_t index;

    memset(OS_impl_queue_table, 0, sizeof(OS_impl_queue_table));
    POSIX_GlobalVars.TruncateQueueDepth = OSAL_BLOCKCOUNT_C(0);
    for (index = 0; index < OS_MAX_QUEUES; ++index)
    {
        OS_impl_queue_internal_record_t *impl = &OS_impl_queue_table[index];
        if (pthread_mutex_init(&impl->mutex, NULL) != 0)
        {
            return OS_ERROR;
        }
        if (pthread_cond_init(&impl->not_empty, NULL) != 0)
        {
            pthread_mutex_destroy(&impl->mutex);
            return OS_ERROR;
        }
        if (pthread_cond_init(&impl->no_waiters, NULL) != 0)
        {
            pthread_cond_destroy(&impl->not_empty);
            pthread_mutex_destroy(&impl->mutex);
            return OS_ERROR;
        }
    }
    return OS_SUCCESS;
}

int32 OS_QueueCreate_Impl(const OS_object_token_t *token, uint32 flags)
{
    OS_impl_queue_internal_record_t *impl = OS_OBJECT_TABLE_GET(OS_impl_queue_table, *token);
    OS_queue_internal_record_t *queue = OS_OBJECT_TABLE_GET(OS_queue_table, *token);
    unsigned char *messages;
    size_t *lengths;
    size_t depth = queue->max_depth;
    size_t message_size = queue->max_size;

    (void)flags;
    if (depth == 0 || message_size == 0 || depth > SIZE_MAX / message_size || depth > SIZE_MAX / sizeof(*lengths))
    {
        return OS_QUEUE_INVALID_SIZE;
    }
    messages = calloc(depth, message_size);
    lengths = calloc(depth, sizeof(*lengths));
    if (messages == NULL || lengths == NULL)
    {
        free(messages);
        free(lengths);
        return OS_ERROR;
    }

    if (pthread_mutex_lock(&impl->mutex) != 0)
    {
        free(messages);
        free(lengths);
        return OS_ERROR;
    }
    if (impl->open)
    {
        pthread_mutex_unlock(&impl->mutex);
        free(messages);
        free(lengths);
        return OS_ERROR;
    }
    impl->messages = messages;
    impl->lengths = lengths;
    impl->message_size = message_size;
    impl->depth = depth;
    impl->head = 0;
    impl->tail = 0;
    impl->count = 0;
    impl->waiters = 0;
    impl->object_id = OS_ObjectIdFromToken(token);
    impl->open = true;
    pthread_mutex_unlock(&impl->mutex);
    return OS_SUCCESS;
}

int32 OS_QueueDelete_Impl(const OS_object_token_t *token)
{
    OS_impl_queue_internal_record_t *impl = OS_OBJECT_TABLE_GET(OS_impl_queue_table, *token);

    if (pthread_mutex_lock(&impl->mutex) != 0)
    {
        return OS_ERROR;
    }
    if (!impl->open || !OS_ObjectIdEqual(impl->object_id, OS_ObjectIdFromToken(token)))
    {
        pthread_mutex_unlock(&impl->mutex);
        return OS_ERROR;
    }
    impl->open = false;
    pthread_cond_broadcast(&impl->not_empty);
    while (impl->waiters > 0)
    {
        if (pthread_cond_wait(&impl->no_waiters, &impl->mutex) != 0)
        {
            pthread_mutex_unlock(&impl->mutex);
            return OS_ERROR;
        }
    }
    free(impl->messages);
    free(impl->lengths);
    impl->messages = NULL;
    impl->lengths = NULL;
    impl->count = 0;
    pthread_mutex_unlock(&impl->mutex);
    return OS_SUCCESS;
}

int32 OS_QueueGet_Impl(const OS_object_token_t *token, void *data, size_t size, size_t *size_copied, int32 timeout)
{
    OS_impl_queue_internal_record_t *impl = OS_OBJECT_TABLE_GET(OS_impl_queue_table, *token);
    struct timespec deadline;
    int wait_result = 0;
    int32 result = OS_ERROR;

    *size_copied = 0;
    if (timeout != OS_PEND && timeout != OS_CHECK)
    {
        OS_Posix_CompAbsDelayTime(timeout, &deadline);
    }
    if (pthread_mutex_lock(&impl->mutex) != 0)
    {
        return OS_ERROR;
    }
    if (!impl->open || !OS_ObjectIdEqual(impl->object_id, OS_ObjectIdFromToken(token)))
    {
        pthread_mutex_unlock(&impl->mutex);
        return OS_ERROR;
    }
    ++impl->waiters;
    pthread_cleanup_push(MK3_QueueWaitCleanup, impl);
    while (impl->open && impl->count == 0 && timeout != OS_CHECK && wait_result == 0)
    {
        if (timeout == OS_PEND)
        {
            wait_result = pthread_cond_wait(&impl->not_empty, &impl->mutex);
        }
        else
        {
            wait_result = pthread_cond_timedwait(&impl->not_empty, &impl->mutex, &deadline);
        }
    }
    if (!impl->open || !OS_ObjectIdEqual(impl->object_id, OS_ObjectIdFromToken(token)))
    {
        result = OS_ERROR;
    }
    else if (impl->count > 0)
    {
        size_t copied = impl->lengths[impl->head];
        if (size < copied)
        {
            result = OS_QUEUE_INVALID_SIZE;
        }
        else
        {
            memcpy(data, impl->messages + impl->head * impl->message_size, copied);
            impl->head = (impl->head + 1) % impl->depth;
            --impl->count;
            *size_copied = copied;
            result = OS_SUCCESS;
        }
    }
    else if (timeout == OS_CHECK)
    {
        result = OS_QUEUE_EMPTY;
    }
    else if (wait_result == ETIMEDOUT)
    {
        result = OS_QUEUE_TIMEOUT;
    }

    pthread_cleanup_pop(1);
    return result;
}

int32 OS_QueuePut_Impl(const OS_object_token_t *token, const void *data, size_t size, uint32 flags)
{
    OS_impl_queue_internal_record_t *impl = OS_OBJECT_TABLE_GET(OS_impl_queue_table, *token);
    int32 result;

    (void)flags;
    if (pthread_mutex_lock(&impl->mutex) != 0)
    {
        return OS_ERROR;
    }
    if (!impl->open || !OS_ObjectIdEqual(impl->object_id, OS_ObjectIdFromToken(token)))
    {
        result = OS_ERROR;
    }
    else if (size > impl->message_size)
    {
        result = OS_QUEUE_INVALID_SIZE;
    }
    else if (impl->count >= impl->depth)
    {
        result = OS_QUEUE_FULL;
    }
    else
    {
        memcpy(impl->messages + impl->tail * impl->message_size, data, size);
        impl->lengths[impl->tail] = size;
        impl->tail = (impl->tail + 1) % impl->depth;
        ++impl->count;
        pthread_cond_signal(&impl->not_empty);
        result = OS_SUCCESS;
    }
    pthread_mutex_unlock(&impl->mutex);
    return result;
}
