#ifndef BITSET_H
#define BITSET_H





#ifdef __cplusplus
extern "C" {
#endif

    #ifndef _POSIX_C_SOURCE
        #define _POSIX_C_SOURCE 200809L
    #endif

    #ifndef _GNU_SOURCE
        #define _GNU_SOURCE
    #endif
    #include <limits.h>
    #include <stdlib.h>
    #include <string.h>
    #include <stdio.h>
    #include <stdint.h>
#ifdef __cplusplus
}
#endif



struct bitset {
    uint64_t size;
    uint64_t cap;
};


#define bitset_mask(__bit) \
    (1ULL << ((__bit) % CHAR_BIT))

#define bitset_alocsz(__size) \
    (__size) / CHAR_BIT + (((__size) % CHAR_BIT) != 0)


#define bitset_create(__size) ({                                            \
    size_t bitset_sz = bitset_alocsz(__size);                               \
    char *base = (char*)calloc(1, bitset_sz + sizeof(struct bitset));       \
    ((struct bitset*)base)->cap = (__size);                                 \
    ((struct bitset*)base);                                                 \
})


#define bitset_check(__bitset, __bit) ({                                    \
    int __val = -1;                                                         \
    if (0 <= (__bit) && (__bit) < (__bitset)->cap) {                        \
        char *bit_arr = ((char*)(__bitset)) + sizeof(struct bitset);        \
        __val = (bit_arr[(__bit) / CHAR_BIT] & bitset_mask(__bit)) != 0;    \
    }                                                                       \
    __val;                                                                  \
})

#define bitset_set(__bitset, __bit) do {                                    \
    if (0 <= (__bit) && (__bit) < (__bitset)->cap) {                        \
        char *bit_arr = ((char*)(__bitset)) + sizeof(struct bitset);        \
       __bitset->size += (bitset_check(__bitset, __bit)) == 0;              \
        bit_arr[(__bit) / CHAR_BIT] |= bitset_mask(__bit);                  \
    }                                                                       \
} while (0)


#define bitset_clear(__bitset, __bit) do {                                  \
    if (0 <= (__bit) && (__bit) < (__bitset)->cap) {                        \
        char *bit_arr = ((char*)(__bitset)) + sizeof(struct bitset);        \
        __bitset->size -= (bitset_check(__bitset, __bit)) == 1;             \
        bit_arr[(__bit) / CHAR_BIT] &= ~(uint64_t){bitset_mask(__bit)};     \
    }                                                                       \
} while (0)


#define bitset_get_word(__bitset, __start, __end) ({                        \
    uint64_t __word    = 0;                                                 \
    if (0 <= (__start) && (__end) < (__bitset)->cap) {                      \
        uint64_t __upper = __end - __start + 1;                             \
        for (uint64_t __i = __start, __j = 0; __j < __upper; ++__i, ++__j)  \
            __word += (bitset_check(__bitset, __i) << __i);                 \
    }                                                                       \
    __word;                                                                 \
})


#define bitset_reset(__bitset) do {                                         \
    uint64_t bitset_sz = bitset_alocsz(((struct bitset*)(__bitset))->cap);  \
    char *bit_arr = ((char*)(__bitset)) + sizeof(struct bitset);            \
    memset(bit_arr, 0, bitset_sz);                                          \
    ((struct bitset*)(__bitset))->size = 0;                                 \
} while (0)


#define bitset_destroy(__bitset) do {   \
    free((__bitset));                   \
} while (0)


#endif /* BITSET_H */