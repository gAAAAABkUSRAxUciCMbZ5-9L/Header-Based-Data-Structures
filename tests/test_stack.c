#include "acutest.h"
#include "stack.h"


void test_stack_create(void) {
    int **ptr = stack_create(int*, STACK_MIN_CAP * 2, free, true);
    TEST_ASSERT(ptr != NULL);
    TEST_ASSERT(stack_get_destructor(&ptr) == free);
    TEST_ASSERT(stack_get_cap(&ptr) == STACK_MIN_CAP * 2);
    TEST_ASSERT(stack_get_size(&ptr) == 0);
    TEST_ASSERT(stack_get_flag(&ptr));
    stack_destroy(&ptr);
}


void test_stack_insert(void) {
    int N = 10000;
    int **ptr = stack_create(int*, STACK_MIN_CAP, NULL, true);
    int *array = malloc(sizeof(int) * N);

    for (int i = 0; i < N; ++i) {
        stack_insert_top(&ptr, &array[i]);
        TEST_ASSERT(stack_get_size(&ptr) == i + 1);
        TEST_ASSERT(stack_get_top(ptr) == &array[i]);
    }
    stack_destroy(&ptr);
    free(array);
}


void test_stack_delete(void) {
    int N = 10000;
    int **ptr = stack_create(int*, STACK_MIN_CAP, NULL, true);
    int *array = malloc(sizeof(int) * N);

    for (int i = 0; i < N; ++i)
        stack_insert_top(&ptr, &array[i]);

    for (int i = N - 1; i >= 0; --i) {
        TEST_ASSERT(stack_get_top(ptr) == &array[i]);
        stack_remove_top(&ptr);
        TEST_ASSERT(stack_get_size(&ptr) == i);
    }
    stack_destroy(&ptr);
    free(array);
}




TEST_LIST = {
    "test_stack_create" , test_stack_create,
    "test_stack_insert",  test_stack_insert,
    "test_stack_delete",  test_stack_delete,
    NULL,                  NULL
};