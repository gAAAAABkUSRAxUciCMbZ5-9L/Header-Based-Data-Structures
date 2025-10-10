#include "acutest.h"
#include "queue.h"


void test_queue_create(void) {
    int **ptr = (int**)queue_create(sizeof(int*), QUEUE_MIN_CAP * 2, free, true);
    TEST_ASSERT(ptr != NULL);
    TEST_ASSERT(queue_get_destructor(&ptr) == free);
    TEST_ASSERT(queue_get_cap(&ptr) == QUEUE_MIN_CAP * 2);
    TEST_ASSERT(queue_get_size(&ptr) == 0);
    TEST_ASSERT(queue_get_flag(&ptr));
    queue_destroy(&ptr);
}


void test_queue_insert(void) {
    int N = 10000;
    int **ptr = (int**)queue_create(sizeof(int*), N, NULL, true);
    int *array = malloc(sizeof(int) * N);

    for (size_t i = 0UL; i < N; ++i) {
        queue_insert_back(&ptr, &array[i]);
        TEST_ASSERT(queue_get_size(&ptr) == i + 1);
        TEST_ASSERT(ptr[0] == &array[0]);
        TEST_ASSERT(ptr[i] == &array[i]);
    }
    queue_destroy(&ptr);
    free(array);
}


void test_queue_delete(void) {
    int N = 10000;
    int **ptr = (int**)queue_create(sizeof(int*), N, NULL, true);
    int *array = malloc(sizeof(int) * N);

    for (size_t i = 0UL; i < N; ++i)
        queue_insert_back(&ptr, &array[i]);

    for (size_t i = 0UL; i < N; ++i) {
        TEST_ASSERT(ptr[0] == &array[i]);
        queue_remove_front(&ptr);
        TEST_ASSERT(queue_get_size(&ptr) == N - 1 - i);
    }
    queue_destroy(&ptr);
    free(array);
}


TEST_LIST = {
    "test_queue_create" , test_queue_create,
    "test_queue_insert",  test_queue_insert,
    "test_queue_delete",  test_queue_delete,
    NULL,                  NULL
};