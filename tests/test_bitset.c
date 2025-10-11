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
    bitset_reset(bitset);

    bitset_set(bitset, 0);
    bitset_set(bitset, 4);
    bitset_set(bitset, 7);

    uint64_t target1 = (1 << 0) + (1 << 4) + (1 << 7);
    uint64_t target2 = (1 << 0) + (1 << 4);
    uint64_t target3 = (1 << 4) + (1 << 7);

    TEST_ASSERT(bitset_get_word(bitset, 0, 7) == target1);
    TEST_ASSERT(bitset_get_word(bitset, 0, 4) == target2);
    TEST_ASSERT(bitset_get_word(bitset, 4, 7) == target3);

    bitset_destroy(bitset);
}


TEST_LIST = {
    "test_init_bitset",   test_init_bitset,
    "test_toggle_bitset", test_toggle_bitset,
    NULL,                 NULL 
};
