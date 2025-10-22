#ifndef HEAP_H
#define HEAP_H


#ifdef __cplusplus
extern "C" {
#endif
    #ifndef _POSIX_C_SOURCE
        #define _POSIX_C_SOURCE 200809L
    #endif   

    #ifndef _GNU_SOURCE
        #define _GNU_SOURCE
    #endif
    #include "vector.h"

#ifdef __cplusplus
}
#endif

#define HEAP_MIN_CAP     64

#define LEFT_CHILD(i)    ((i << 1) + 1)
#define RIGHT_CHILD(i)   (LEFT_CHILD(i) + 1) 
#define PARENT(i)        ((i - 1) >> 1)



struct heap {
    int (*cmp_func)(void *a, void *b);
    char *entries;
};



#define heap_calc_cap(__init_cap) ({                                            \
    __label__ __final;                                                          \
    uint32_t __final_cap = 1;                                                   \
    bool is_pow2 = (__init_cap) > 1                                             \
        ? ((__init_cap) & ((__init_cap) - 1)) == 0                              \
        : false;                                                                \
    if (is_pow2 || __init_cap <= 1) goto __final;                               \
    while (__final_cap < (__init_cap)) __final_cap <<= 1;                       \
    __final:                                                                    \
        (__final_cap == 1 ? is_pow2 ? __init_cap : HEAP_MIN_CAP : __final_cap); \
})


#define heap_get_size(__heap_ptr) ({            \
    vector_get_size(&(__heap_ptr)->entries);    \
})

#define heap_get_cap(__heap_ptr) ({         \
    vector_get_cap(&(__heap_ptr)->entries); \
})

#define heap_get_entry_size(__heap_ptr) ({          \
    vector_get_entry_size(&(__heap_ptr)->entries);  \
})


#define heap_shift_down(__heap_ptr, __pos) do {                                                         \
    __label__ recurse;                                                                                  \
    uint64_t __heap_sz       = heap_get_size(__heap_ptr);                                               \
    uint64_t __heap_entry_sz = heap_get_entry_size(__heap_ptr);                                         \
    char    *__temp_buffer   = (char*)malloc(__heap_entry_sz);                                          \
    uint64_t __left          = LEFT_CHILD((__pos));                                                     \
    uint64_t __right         = RIGHT_CHILD((__pos));                                                    \
                                                                                                        \
    recurse:                                                                                            \
    if (__left < __heap_sz) {                                                                           \
        uint64_t __max = __left;                                                                        \
        int __res = (__heap_ptr)->cmp_func((__heap_ptr)->entries + __left * __heap_entry_sz,            \
                                           (__heap_ptr)->entries + __right * __heap_entry_sz);          \
        if (__right < __heap_sz && __res < 0) __max = __right;                                          \
                                                                                                        \
        __res = (__heap_ptr)->cmp_func((__heap_ptr)->entries + __pos * __heap_entry_sz,                 \
                                       (__heap_ptr)->entries + __max * __heap_entry_sz);                \
        if (__res < 0) {                                                                                \
            memcpy(__temp_buffer, (__heap_ptr)->entries + __pos * __heap_entry_sz, __heap_entry_sz);    \
            memcpy((__heap_ptr)->entries + __pos * __heap_entry_sz,                                     \
                   (__heap_ptr)->entries + __max * __heap_entry_sz, __heap_entry_sz);                   \
            memcpy((__heap_ptr)->entries + __max * __heap_entry_sz, __temp_buffer, __heap_entry_sz);    \
            __pos = __max;                                                                              \
            __left  = LEFT_CHILD(__max);                                                                \
            __right = RIGHT_CHILD(__max);                                                               \
            goto recurse;                                                                               \
        }                                                                                               \
    }                                                                                                   \
    free(__temp_buffer);                                                                                \
} while (0)

#define heap_init(__heap_ptr, __entries_ptr, __entries_size) do {                                   \
    uint64_t __heap_entry_sz = heap_get_entry_size(__heap_ptr);                                     \
    uint64_t __heap_cap      = heap_get_cap(__heap_ptr);                                            \
    if (__heap_cap >= __entries_size) {                                                             \
        memcpy((__heap_ptr)->entries, (char*)(__entries_ptr), (__entries_size) * __heap_entry_sz);  \
        vector_set_size(&(__heap_ptr)->entries, (__entries_size));                                  \
        for (int64_t i = (__entries_size) / 2 - 1; i >= 0; --i) {                                   \
            heap_shift_down(__heap_ptr, i);                                                         \
        }                                                                                           \
    }                                                                                               \
} while (0)



#define heap_shift_up(__heap_ptr, __pos) do {                                                   \
    uint64_t pos__ = (__pos);                                                                   \
    uint64_t __heap_entry_sz = heap_get_entry_size(__heap_ptr);                                 \
    if (pos__ > 0) {                                                                            \
        uint64_t __parent_id = PARENT(pos__);                                                   \
        char *__temp_buffer = (char*)malloc(__heap_entry_sz);                                   \
        char *__entries = (__heap_ptr)->entries;                                                \
        while (pos__ > 0 &&                                                                     \
            (__heap_ptr)->cmp_func(__entries + __parent_id * __heap_entry_sz,                   \
                                   __entries + pos__ * __heap_entry_sz) < 0) {                  \
            memcpy(__temp_buffer, __entries + __parent_id * __heap_entry_sz, __heap_entry_sz);  \
            memcpy(__entries + __parent_id * __heap_entry_sz,                                   \
                   __entries + pos__ * __heap_entry_sz, __heap_entry_sz);                       \
            memcpy(__entries + pos__ * __heap_entry_sz, __temp_buffer, __heap_entry_sz);        \
            pos__ = __parent_id;                                                                \
            __parent_id = PARENT(pos__);                                                        \
        }                                                                                       \
        free(__temp_buffer);                                                                    \
    }                                                                                           \
} while (0)


#define heap_create(__type, __init_cap, __destructor, __cmp_func, __ptr_flag) ({                \
    uint64_t __final_cap = heap_calc_cap(__init_cap);                                           \
    struct heap *__heap = (struct heap*)malloc(sizeof(struct heap));                            \
    __heap->entries = (char*)vector_create(__type, __final_cap, __destructor, __ptr_flag);      \
    __heap->cmp_func = (__cmp_func);                                                            \
    __heap;                                                                                     \
})


#define heap_destroy(__heap_ptr) do {       \
    vector_destroy(&(__heap_ptr)->entries); \
    free(__heap_ptr);                       \
} while (0)


#define heap_insert(__heap_ptr, __entry) do {                   \
    uint64_t __heap_sz = heap_get_size(__heap_ptr);             \
    vector_insert(&(__heap_ptr)->entries, __entry);             \
    heap_shift_up(__heap_ptr, __heap_sz);                       \
} while (0)


#define heap_delete(__heap_ptr, __index) do {                                                       \
    uint64_t __heap_sz       = heap_get_size(__heap_ptr);                                           \
    uint64_t __heap_entry_sz = heap_get_entry_size(__heap_ptr);                                     \
    uint64_t index__ = (__index);                                                                   \
    if (0 <= (index__) && (index__) < __heap_sz) {                                                  \
        char *temp = (char*)malloc(__heap_entry_sz);                                                \
        memcpy(temp, (__heap_ptr)->entries + __heap_entry_sz * index__, __heap_entry_sz);           \
        memcpy((__heap_ptr)->entries + __heap_entry_sz * index__,                                   \
               (__heap_ptr)->entries + __heap_entry_sz * (__heap_sz - 1), __heap_entry_sz);         \
        memcpy((__heap_ptr)->entries + __heap_entry_sz * (__heap_sz - 1), temp, __heap_entry_sz);   \
        vector_delete(&((__heap_ptr)->entries), __heap_sz - 1);                                     \
        heap_shift_down(__heap_ptr, index__);                                                       \
        heap_shift_up(__heap_ptr, index__);                                                         \
        free(temp);                                                                                 \
    }                                                                                               \
} while (0)


#define heap_delete_top(__heap_ptr) do {    \
    heap_delete(__heap_ptr, 0);             \
} while (0)


#define heap_get_top(__heap_ptr) ({                         \
    uint64_t __heap_sz = heap_get_size(__heap_ptr);         \
    (__heap_sz == 0 ? NULL : (__heap_ptr)->entries);        \
})


#endif /* HEAP_H */
