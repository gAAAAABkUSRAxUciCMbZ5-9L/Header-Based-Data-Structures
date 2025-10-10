#include "acutest.h"
#include "list.h"



static int cmp_ints(void *a, void *b) {
    int aa = *(int*)a;
    int bb = *(int*)b;
    return (aa > bb) - (aa < bb);
}


void test_dllist_create(void) {
    struct dll_list *list = dll_create(int*, free, true);
    TEST_ASSERT(list != NULL);
    TEST_ASSERT(list->destructor == free);
    TEST_ASSERT(list->is_ptr);
    TEST_ASSERT(list->entry_size == sizeof(int*));
    TEST_ASSERT(list->size == 0);
    TEST_ASSERT(list->last == NULL);
    TEST_ASSERT(list->first == NULL);
    dll_destroy(list);
}


void test_dllist_insert(void) {
    int N = 10000;
    struct dll_list *list = dll_create(int*, NULL, true);
    int *array = malloc(N * sizeof(int));

    for (int i = 0; i < N; ++i) {
        dll_insert_start(list, &array[i]);
        TEST_ASSERT(list->size == i + 1);
        TEST_ASSERT(*(int**)list->first->entry == &array[i]);
        TEST_ASSERT((list->first == list->last) ^ (i != 0));
    }

    struct dll_node *node = list->first;
    for (int i = N - 1; i >= 0; --i) {
        TEST_ASSERT(*(int**)node->entry == &array[i]);
        TEST_ASSERT((node->prev == NULL) ^ (i != N - 1));
        TEST_ASSERT((node->next == NULL) ^ (i != 0));
        if (0 < i && i < N - 1) {
            TEST_ASSERT(*(int**)node->prev->entry == &array[i + 1]);
            TEST_ASSERT(*(int**)node->next->entry == &array[i - 1]);
        }
        node = node->next;
    }
    dll_destroy(list);

    list = dll_create(int*, NULL, true);
    for (int i = 0; i < N; ++i) {
        dll_insert_end(list, &array[i]);
        TEST_ASSERT(*(int**)list->last->entry == &array[i]);
        TEST_ASSERT(list->size == i + 1);
        TEST_ASSERT((list->first == list->last) ^ (i != 0));
    }

    node = list->first;
    for (int i = 0; i < N; ++i) {
        TEST_ASSERT(*(int**)node->entry == &array[i]);
        TEST_ASSERT((node->prev == NULL) ^ (i != 0));
        TEST_ASSERT((node->next == NULL) ^ (i != N - 1));
        if (0 < i && i < N - 1) {
            TEST_ASSERT(*(int**)node->prev->entry == &array[i - 1]);
            TEST_ASSERT(*(int**)node->next->entry == &array[i + 1]);
        }
        node = node->next;
    }

    int *dummy_ptr = (int*)((uintptr_t)0xDEADBEEF);
    dll_insert_after(list, list->first, dummy_ptr);
    dll_insert_before(list, list->last, dummy_ptr);
    TEST_ASSERT(*(int**)list->first->next->entry == dummy_ptr);
    TEST_ASSERT(*(int**)list->last->prev->entry == dummy_ptr);

    dll_destroy(list);
    free(array);
}


void test_dllist_delete(void) {
    int N = 1000;
    struct dll_list *list = dll_create(int, NULL, false);

    for (int i = 0; i < N; ++i)
        dll_insert_start(list, i);

        
    TEST_ASSERT(list->size == N);
    for (int i = N - 1; i >= 0; --i) {
        TEST_ASSERT(*(int*)list->first->entry == i);
        dll_delete(list, list->first);
        TEST_ASSERT(list->size == i);
    }

    for (int i = 0; i < N; ++i)
        dll_insert_end(list, i);

    TEST_ASSERT(list->size == N);
    for (int i = N - 1; i >= 0; --i) {
        TEST_ASSERT(*(int*)list->last->entry == i);
        dll_delete(list, list->last);
        TEST_ASSERT(list->size == i);
    }
    dll_destroy(list);
}


void test_dllist_combine(void) {
    int N = 1000;
    struct dll_list *list = dll_create(int, NULL, false);
    
    for (int i = 0; i < N; ++i) {
        dll_insert_end(list, i);
        struct dll_node *find = dll_find(list, i, cmp_ints);
        TEST_ASSERT(*(int*)find->entry == i);
    }
    TEST_ASSERT(dll_find(list, N, cmp_ints) == NULL);

    struct dll_node *find = dll_find(list, N - 2, cmp_ints);
    TEST_ASSERT(find != NULL);
    TEST_ASSERT(*(int*)find->entry == N - 2);
    TEST_ASSERT(*(int*)find->next->entry == N - 1);
    TEST_ASSERT(*(int*)find->prev->entry == N - 3);

    dll_insert_after(list, find, 2025);
    dll_insert_before(list, find, 2024);
    TEST_ASSERT(*(int*)find->entry == N - 2);
    TEST_ASSERT(*(int*)find->next->entry == 2025);
    TEST_ASSERT(*(int*)find->prev->entry == 2024);

    dll_delete(list, find);
    TEST_ASSERT(dll_find(list, N - 2, cmp_ints) == NULL);
    find = dll_find(list, 2024, cmp_ints);
    TEST_ASSERT(find != NULL);
    TEST_ASSERT(*(int*)find->entry == 2024);
    TEST_ASSERT(*(int*)find->next->entry == 2025);

    dll_destroy(list);
}



TEST_LIST = {
    "test_dllist_create" , test_dllist_create,
    "test_dllist_insert",  test_dllist_insert,
    "test_dllist_delete",  test_dllist_delete,
    "test_dllist_combine", test_dllist_combine,
    NULL,                  NULL
};
