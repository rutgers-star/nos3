#ifndef MK3_OS_IMPL_QUEUES_H
#define MK3_OS_IMPL_QUEUES_H

#include "osconfig.h"
#include <pthread.h>
#include <stdbool.h>
#include <stddef.h>

typedef struct
{
    pthread_mutex_t mutex;
    pthread_cond_t not_empty;
    pthread_cond_t no_waiters;
    unsigned char *messages;
    size_t *lengths;
    size_t message_size;
    size_t depth;
    size_t head;
    size_t tail;
    size_t count;
    size_t waiters;
    osal_id_t object_id;
    bool open;
} OS_impl_queue_internal_record_t;

extern OS_impl_queue_internal_record_t OS_impl_queue_table[OS_MAX_QUEUES];

#endif
