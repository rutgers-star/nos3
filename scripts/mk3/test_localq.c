/* Functional smoke test for the MK3 OSAL process-local queue backend. */
#include "osapi.h"
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

static osal_id_t queue_id;
static int32 wait_result;

static void *waiter(void *unused)
{
    char buffer[8];
    size_t copied = 0;
    (void)unused;
    wait_result = OS_QueueGet(queue_id, buffer, sizeof(buffer), &copied, OS_PEND);
    return NULL;
}

#define CHECK(call, expected) do { int32 rc = (call); if (rc != (expected)) { \
    fprintf(stderr, "%s returned %ld, expected %ld\n", #call, (long)rc, (long)(expected)); return 1; \
} } while (0)

static int run_test(void)
{
    char buffer[8] = {0};
    size_t copied = 99;
    pthread_t thread;

    CHECK(OS_API_Init(), OS_SUCCESS);
    CHECK(OS_QueueCreate(&queue_id, "mk3-smoke", 2, sizeof(buffer), 0), OS_SUCCESS);
    CHECK(OS_QueueGet(queue_id, buffer, sizeof(buffer), &copied, OS_CHECK), OS_QUEUE_EMPTY);
    CHECK(OS_QueueGet(queue_id, buffer, sizeof(buffer), &copied, 30), OS_QUEUE_TIMEOUT);
    CHECK(OS_QueuePut(queue_id, "one", 4, 0), OS_SUCCESS);
    CHECK(OS_QueuePut(queue_id, "two", 4, 0), OS_SUCCESS);
    CHECK(OS_QueuePut(queue_id, "full", 5, 0), OS_QUEUE_FULL);
    CHECK(OS_QueueGet(queue_id, buffer, sizeof(buffer), &copied, OS_CHECK), OS_SUCCESS);
    if (copied != 4 || strcmp(buffer, "one") != 0) return 2;
    CHECK(OS_QueueGet(queue_id, buffer, sizeof(buffer), &copied, OS_CHECK), OS_SUCCESS);
    if (copied != 4 || strcmp(buffer, "two") != 0) return 3;
    if (pthread_create(&thread, NULL, waiter, NULL) != 0) return 4;
    usleep(100000);
    CHECK(OS_QueueDelete(queue_id), OS_SUCCESS);
    if (pthread_join(thread, NULL) != 0 || wait_result != OS_ERROR) return 5;
    CHECK(OS_QueueCreate(&queue_id, "mk3-smoke-reuse", 2, sizeof(buffer), 0), OS_SUCCESS);
    if (pthread_create(&thread, NULL, waiter, NULL) != 0) return 6;
    usleep(100000);
    if (pthread_cancel(thread) != 0 || pthread_join(thread, NULL) != 0) return 7;
    CHECK(OS_QueueDelete(queue_id), OS_SUCCESS);
    puts("MK3 local queue smoke test passed");
    return 0;
}

void OS_Application_Startup(void)
{
    exit(run_test());
}

void OS_Application_Run(void)
{
}
