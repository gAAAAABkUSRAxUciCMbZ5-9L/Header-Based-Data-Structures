#include <sys/time.h>

#include "heap.h"
#include "acutest.h"

#define ARRAY_SIZE(arr) (sizeof(arr) / sizeof((arr)[0]))


static int cmp_incr_ints(void *a, void *b) {
    int aa = *(int*)a;
    int bb = *(int*)b;
    return (aa > bb) - (aa < bb);
}


static int cmp_decr_ints(void *a, void *b) {
    int aa = *(int*)a;
    int bb = *(int*)b;
    return (bb > aa) - (bb < aa);
}


static int cmp_point_ints(void *a, void *b) {
    int *aa = *(int**)a;
    int *bb = *(int**)b;
    return cmp_incr_ints(aa, bb);
}


static int *create_int(int val) {
    int *ptr = malloc(sizeof(int));
    *ptr = val;
    return ptr;
}


static void shuffle(int **array, size_t n) {    
    struct timeval tv;
    gettimeofday(&tv, NULL);
    srand48(tv.tv_usec);

    for (size_t i = n - 1; i > 0; --i) {
        size_t j = (size_t)(drand48() * (i + 1));
        int *temp = array[j];
        array[j] = array[i];
        array[i] = temp;
    }
}


static bool is_valid_heap(struct heap *heap) {
    uint64_t heap_sz  = heap_get_size(heap);
    uint64_t entry_sz = heap_get_entry_size(heap); 
    int64_t bound = heap_sz / 2 - 1;
    for (int64_t i = 0; i <= bound; ++i) {
        uint64_t left_id  = LEFT_CHILD(i);
        uint64_t right_id = RIGHT_CHILD(i);
        char *left_entry  = heap->entries + left_id * entry_sz;
        char *right_entry = heap->entries + right_id * entry_sz;
        char *root_entry  = heap->entries + i * entry_sz;
        
        if (left_id <= heap_sz - 1 &&
            heap->cmp_func(root_entry, left_entry) < 0) 
            return false;
        
        if (right_id <= heap_sz - 1 &&
            heap->cmp_func(root_entry, right_entry) < 0)
            return false;
    }
    return true;
}



void test_heap_create(void) {
    struct heap *h1 = heap_create(int, HEAP_MIN_CAP, NULL,
                                  cmp_decr_ints, false);
    struct heap *h2 = heap_create(int, HEAP_MIN_CAP, NULL,
                                  cmp_incr_ints, false);

    TEST_ASSERT(h1 != NULL);
    TEST_ASSERT(h1->cmp_func == cmp_decr_ints);
    TEST_ASSERT(heap_get_size(h1) == 0);
    TEST_ASSERT(heap_get_cap(h1) == HEAP_MIN_CAP);
    TEST_ASSERT(heap_get_top(h1) == NULL);


    int nums[] = { -100, 20, -1821, 0 };
    heap_init(h1, nums, ARRAY_SIZE(nums));
    heap_init(h2, nums, ARRAY_SIZE(nums));
    TEST_ASSERT(is_valid_heap(h1));
    TEST_ASSERT(is_valid_heap(h2));
    TEST_ASSERT(heap_get_size(h1) == ARRAY_SIZE(nums));
    TEST_ASSERT(heap_get_size(h2) == ARRAY_SIZE(nums));


    TEST_ASSERT(*(int*)heap_get_top(h1) == -1821);
    TEST_ASSERT(*(int*)heap_get_top(h2) == 20);

    heap_delete_top(h1);
    heap_delete_top(h2);
    TEST_ASSERT(is_valid_heap(h1));
    TEST_ASSERT(is_valid_heap(h2));
    TEST_ASSERT(*(int*)heap_get_top(h1) == -100);
    TEST_ASSERT(*(int*)heap_get_top(h2) == 0);
    
    heap_destroy(h1);
    heap_destroy(h2);
}


void test_heap_insert(void) {
    int N = 10000;
    int *ptr = malloc(sizeof(int) * N);
    struct heap *heap = heap_create(int*, N, NULL, cmp_point_ints, true);

    for (int i = 0; i < N; ++i) {
        ptr[i] = i;
        heap_insert(heap, (int*){&ptr[i]});
        TEST_ASSERT(heap_get_size(heap) == i + 1);
        TEST_ASSERT(*(int**)heap_get_top(heap) == &ptr[i]);
        TEST_ASSERT(is_valid_heap(heap));
    }
    heap_destroy(heap);
    free(ptr);
}


void test_heap_delete(void) {
    int N = 10000;
    struct heap *heap = heap_create(int*, N, free, cmp_point_ints, true);

    int **ptr = malloc(N * sizeof(int*));
    int **copy_ptr = malloc(N * sizeof(int*));
    for (size_t i = 0UL; i < N; ++i)
        ptr[i] = create_int(i);
    
    memcpy(copy_ptr, ptr, N * sizeof(int*));

    shuffle(ptr, N);
    for (size_t i = 0UL; i < N; ++i)
        heap_insert(heap, ptr[i]);

    for (int i = N - 1; i >= 0; --i) {
        int **top = (int**)heap_get_top(heap);
        TEST_ASSERT(*top == copy_ptr[i]);
        TEST_ASSERT(**top == *copy_ptr[i]);
        heap_delete_top(heap);
        TEST_ASSERT(is_valid_heap(heap));
        TEST_ASSERT(heap_get_size(heap) == i);
    }
    heap_destroy(heap);
    free(ptr);
    free(copy_ptr);

    heap = heap_create(int, HEAP_MIN_CAP, NULL,
                       cmp_decr_ints, false);
    
    for (int i = N; i >= 0; --i)
        heap_insert(heap, i);

    TEST_ASSERT(is_valid_heap(heap));
    TEST_ASSERT(*(int*)heap_get_top(heap) == 0);
    while (heap_get_size(heap) > 1) {
        size_t index = rand() % (heap_get_size(heap) - 1) + 1;
        heap_delete(heap, index);
        TEST_ASSERT(is_valid_heap(heap));
        TEST_ASSERT(*(int*)heap_get_top(heap) == 0);
    }
    TEST_ASSERT(is_valid_heap(heap));
    TEST_ASSERT(*(int*)heap_get_top(heap) == 0);
    heap_destroy(heap);
}


TEST_LIST = {
    "test_heap_create", test_heap_create,
    "test_heap_insert", test_heap_insert,
    "test_heap_delete", test_heap_delete,
    NULL,               NULL 
};
