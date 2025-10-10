#include "bitset.h"
#include "acutest.h"



void test_init_bitset(void) {
    int N = 32;
    struct bitset *bitset = bitset_create(N);

    TEST_ASSERT(bitset != NULL);
    TEST_ASSERT(bitset->size == 0);
    TEST_ASSERT(bitset->cap == N);
    for (uint64_t i = 0; i < bitset->cap; ++i)
        TEST_ASSERT(!bitset_check(bitset, i));

    bitset_destroy(bitset);
}


void test_toggle_bitset(void) {
    int N = 32;
    struct bitset *bitset = bitset_create(N);


    for (int i = 0; i < bitset->cap; i += 2) {
        TEST_ASSERT(!bitset_check(bitset, i));
        bitset_set(bitset, i);
        TEST_ASSERT(bitset_check(bitset, i));
        TEST_ASSERT(bitset->size == (int){i / 2 + 1});
    }


    uint64_t cur_sz = bitset->size;
    for (int i = 0; i < bitset->cap; i += 2) {
        bitset_set(bitset, i);
        TEST_ASSERT(bitset->size == cur_sz);
    }


    for (int i = 0; i < bitset->cap; ++i)
        TEST_ASSERT(bitset_check(bitset, i) + i % 2);


    for (int i = 0; i < bitset->cap; i += 2) {
        bitset_clear(bitset, i);
        TEST_ASSERT(!bitset_check(bitset, i));
        TEST_ASSERT(bitset->size == --cur_sz);
    }

    for (int i = 0; i < bitset->cap; i += 2) {
        bitset_clear(bitset, i);
        TEST_ASSERT(bitset->size == 0);
    }
    bitset_destroy(bitset);
}


TEST_LIST = {
    "test_init_bitset",   test_init_bitset,
    "test_toggle_bitset", test_toggle_bitset,
    NULL,                 NULL 
};
