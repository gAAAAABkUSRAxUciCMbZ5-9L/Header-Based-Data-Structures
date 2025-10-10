#ifndef MAP_H
#define MAP_H



#ifdef __cplusplus
extern "C" {
#endif

    #ifndef _POSIX_C_SOURCE
        #define _POSIX_C_SOURCE 200809L
    #endif

    #ifndef _GNU_SOURCE
        #define _GNU_SOURCE
    #endif
    #include "bitset.h"
    #include <stdbool.h>
#ifdef __cplusplus
}
#endif


#define MAP_MIN_CAP  128


struct map {
    uint64_t        size;
    uint64_t        cap;
    uint64_t        deleted;
    uint64_t        key_size;
    uint64_t        value_size;
    struct bitset  *bit_array;
    uint64_t       (*hash_func)(void *v);
    int            (*cmp_keys)(void *a, void *b);
    void           (*key_destructor)(void *v);
    void           (*value_destructor)(void *v);
};



struct map_tuple {
    char *key;
    char *value;
};


#define map_calc_cap(__init_cap) ({                                             \
    __label__ __final;                                                          \
    uint32_t __final_cap = 1;                                                   \
    bool is_pow2 = (__init_cap) > 1                                             \
        ? ((__init_cap) & ((__init_cap) - 1)) == 0                              \
        : false;                                                                \
    if (is_pow2 || __init_cap <= 1) goto __final;                               \
    while (__final_cap < (__init_cap)) __final_cap <<= 1;                       \
    __final:                                                                    \
        (__final_cap == 1 ? is_pow2 ? __init_cap : MAP_MIN_CAP : __final_cap);  \
})


#define map_create(__key_size, __value_size, __is_key_ptr, __is_value_ptr, __init_cap,                              \
                   __hash_func, __cmp_keys, __key_destructor, __value_destructor) ({                                \
    uint64_t __final_cap = map_calc_cap(__init_cap);                                                                \
    uint64_t __slot_sz   = __final_cap * ((__value_size) + (__key_size) + sizeof(int16_t));                         \
    char *__base_ptr = (char*)calloc(1, sizeof(struct map) + __slot_sz);                                            \
    ((struct map*)__base_ptr)->cap = __final_cap;                                                                   \
    ((struct map*)__base_ptr)->key_size = (__key_size);                                                             \
    ((struct map*)__base_ptr)->value_size = (__value_size);                                                         \
    ((struct map*)__base_ptr)->hash_func = (__hash_func);                                                           \
    ((struct map*)__base_ptr)->key_destructor = (__key_destructor);                                                 \
    ((struct map*)__base_ptr)->value_destructor = (__value_destructor);                                             \
    ((struct map*)__base_ptr)->cmp_keys = (__cmp_keys);                                                             \
    ((struct map*)__base_ptr)->bit_array = bitset_create(__final_cap + 2);                                          \
    if ((__is_key_ptr)) bitset_set(((struct map*)__base_ptr)->bit_array, __final_cap);                              \
    if ((__is_value_ptr)) bitset_set(((struct map*)__base_ptr)->bit_array, __final_cap + 1);                        \
    ((struct map*)__base_ptr);                                                                                      \
})



#define map_find_spot(__map_ptr, __key, __hash_value, __to_insert) ({                                       \
    char *__data_ptr;                                                                                       \
    memcpy(&__data_ptr, __map_ptr, sizeof(char*));                                                          \
    struct map *__map = (struct map*)__data_ptr;                                                            \
    __data_ptr += sizeof(struct map);                                                                       \
    uint64_t __hash = (__hash_value);                                                                       \
    uint64_t __index = (__hash) & (__map->cap - 1);                                                         \
    int16_t __hash_prefix = (__hash >> 24) & 0xFFFF;                                                        \
    int64_t __temp_index = -1;                                                                              \
                                                                                                            \
    uint64_t __offset = __map->key_size + __map->value_size + sizeof(int16_t);                              \
    char *__temp_ptr = __data_ptr + __offset * __index;                                                     \
    while (1) {                                                                                             \
        if (!bitset_check(__map->bit_array, __index)) break;                                                \
        int16_t __temp_prefix = *(int16_t*)(__temp_ptr + __offset - sizeof(int16_t));                       \
        if (__temp_prefix == -1 && __temp_index == -1) __temp_index = __index;                              \
        if (__temp_prefix == __hash_prefix &&                                                               \
            __map->cmp_keys(__key, __temp_ptr) == 0) {                                                      \
            __temp_index = -1;                                                                              \
            break;                                                                                          \
        }                                                                                                   \
        __index = (__index + 1) & (__map->cap - 1);                                                         \
        __temp_ptr = __data_ptr + __offset * __index;                                                       \
    }                                                                                                       \
    (__temp_index != -1 && (__to_insert) ? __temp_index : __index);                                         \
})


#define map_rehash(__map_ptr, __key, __value) {                                                                                         \
    char *__data_ptr;                                                                                                                   \
    memcpy(&__data_ptr, __map_ptr, sizeof(char*));                                                                                      \
    struct map *__map = (struct map*)__data_ptr;                                                                                        \
    char *__entry_ptr = __data_ptr + sizeof(struct map);                                                                                \
    uint64_t __new_size =                                                                                                               \
        sizeof(struct map) + (__map->key_size + __map->value_size + sizeof(int16_t)) * (__map->cap << 1);                               \
    char *__new_ptr = (char*)calloc(1, __new_size);                                                                                     \
    memcpy(__new_ptr, __map, sizeof(struct map));                                                                                       \
    ((struct map*)__new_ptr)->cap = (__map->cap << 1);                                                                                  \
    ((struct map*)__new_ptr)->deleted = 0;                                                                                              \
    ((struct map*)__new_ptr)->bit_array = bitset_create((__map->cap << 1) + 2);                                                         \
    if (bitset_check(__map->bit_array, __map->cap)) bitset_set(((struct map*)__new_ptr)->bit_array, (__map->cap << 1));                 \
    if (bitset_check(__map->bit_array, __map->cap + 1)) bitset_set(((struct map*)__new_ptr)->bit_array, (__map->cap << 1) + 1);         \
    char *new_entry_ptr = __new_ptr + sizeof(struct map);                                                                               \
    struct __attribute__((__packed__)) tuple { typeof(key__) k; typeof(value__) v; int16_t h; };                                        \
    for (uint64_t i = 0; i < __map->cap; ++i) {                                                                                         \
        if (!bitset_check(__map->bit_array, i) || ((struct tuple*)__entry_ptr)[0].h == -1) {                                            \
            __entry_ptr += __map->key_size + __map->value_size + sizeof(int16_t);                                                       \
            continue;                                                                                                                   \
        }                                                                                                                               \
        uint64_t __hash_val = __map->hash_func((void*)__entry_ptr);                                                                     \
        uint64_t __index = map_find_spot(&__new_ptr, __entry_ptr, __hash_val, true);                                                    \
        ((struct tuple*)new_entry_ptr)[__index] = ((struct tuple*)__entry_ptr)[0];                                                      \
        __entry_ptr += __map->key_size + __map->value_size + sizeof(int16_t);                                                           \
        bitset_set(((struct map*)__new_ptr)->bit_array, __index);                                                                       \
    }                                                                                                                                   \
    char *__base_ptr = (char*)(__map_ptr);                                                                                              \
    memcpy(__base_ptr, &__new_ptr, sizeof(char*));                                                                                      \
    bitset_destroy(__map->bit_array);                                                                                                   \
    free(__map);                                                                                                                        \
} while (0)



#define map_insert(__map_ptr, __key, __value) do {                                                                                      \
    typeof(__key) key__ = (__key);                                                                                                      \
    typeof(__value) value__ = (__value);                                                                                                \
    char *__data_ptr;                                                                                                                   \
    memcpy(&__data_ptr, __map_ptr, sizeof(char*));                                                                                      \
    struct map *__map = (struct map*)__data_ptr;                                                                                        \
    uint64_t __hash_val = __map->hash_func((void*)&key__);                                                                              \
    int64_t __index = map_find_spot(__map_ptr, (void*)&key__, __hash_val, true);                                                        \
    struct __attribute__((__packed__)) tuple { typeof(key__) k; typeof(value__) v; int16_t h; };                                        \
    __data_ptr += sizeof(struct map) + __index * (__map->key_size + __map->value_size + sizeof(int16_t));                               \
    if (bitset_check(__map->bit_array, __index) && ((struct tuple*)__data_ptr)[0].h != -1) {                                            \
        if (__map->key_destructor != NULL) {                                                                                            \
            if (bitset_check(__map->bit_array, __map->cap)) {                                                                           \
                char *key1, *key2, *key3 = (char*)&(key__);                                                                             \
                memcpy(&key1, __data_ptr, sizeof(char*));                                                                               \
                memcpy(&key2, key3, sizeof(char*));                                                                                     \
                if (key1 != key2) {                                                                                                     \
                    __map->key_destructor(key1);                                                                                        \
                }                                                                                                                       \
            } else {                                                                                                                    \
                __map->key_destructor((void*)__data_ptr);                                                                               \
            }                                                                                                                           \
        }                                                                                                                               \
        if (__map->value_destructor != NULL) {                                                                                          \
            if (bitset_check(__map->bit_array, __map->cap + 1)) {                                                                       \
                char *value1, *value2, *value3 = (char*)&value__;                                                                       \
                memcpy(&value1, __data_ptr + __map->key_size, sizeof(char*));                                                           \
                memcpy(&value2, value3, sizeof(char*));                                                                                 \
                if (value1 != value2) {                                                                                                 \
                    __map->value_destructor(value1);                                                                                    \
                }                                                                                                                       \
            } else {                                                                                                                    \
                __map->value_destructor((void*)(__data_ptr + __map->key_size));                                                         \
            }                                                                                                                           \
        }                                                                                                                               \
    }                                                                                                                                   \
    int16_t __hash_prefix = (__hash_val >> 24) & 0xFFFF;                                                                                \
    ((struct tuple*)__data_ptr)[0] = (struct tuple){key__, value__, __hash_prefix};                                                     \
    if (!bitset_check(__map->bit_array, __index)) {                                                                                     \
        __map->size++;                                                                                                                  \
        bitset_set(__map->bit_array, __index);                                                                                          \
    }                                                                                                                                   \
    bool __to_rehash = 100 * (__map->size + __map->deleted) > 85 * __map->cap;                                                          \
    if (__to_rehash) map_rehash(__map_ptr, key__, value__);                                                                             \
} while (0)


#define map_delete(__map_ptr, __key) do {                                                       \
    char *__data_ptr;                                                                           \
    memcpy(&__data_ptr, __map_ptr, sizeof(char*));                                              \
    struct map *__map = (struct map*)__data_ptr;                                                \
    typeof(__key) key__ = (__key);                                                              \
    uint64_t __hash_val = __map->hash_func((void*)&key__);                                      \
    uint64_t __index = map_find_spot(__map_ptr, &key__, __hash_val, false);                     \
    __data_ptr +=                                                                               \
        sizeof(struct map) + __index * (__map->key_size + __map->value_size + sizeof(int16_t)); \
    if (bitset_check(__map->bit_array, __index)) {                                              \
        __map->size--;                                                                          \
        __map->deleted++;                                                                       \
        if (__map->key_destructor != NULL) {                                                    \
            if (bitset_check(__map->bit_array, __map->cap)) {                                   \
                char *entry;                                                                    \
                memcpy(&entry, __data_ptr, __map->key_size);                                    \
                __map->key_destructor(entry);                                                   \
            } else {                                                                            \
                __map->key_destructor((void*)__data_ptr);                                       \
            }                                                                                   \
        }                                                                                       \
        if (__map->value_destructor != NULL) {                                                  \
            char *temp = __data_ptr + __map->key_size;                                          \
            if (bitset_check(__map->bit_array, __map->cap + 1)) {                               \
                char *entry;                                                                    \
                memcpy(&entry, temp, __map->value_size);                                        \
                __map->value_destructor((void*)entry);                                          \
            } else {                                                                            \
                __map->value_destructor((void*)temp);                                           \
            }                                                                                   \
        }                                                                                       \
        memset(__data_ptr + __map->key_size + __map->value_size, -1, sizeof(int16_t));          \
    }                                                                                           \
} while (0)


#define map_find(__map_ptr, __key) ({                                                                       \
    __label__ __to_return;                                                                                  \
    struct map_tuple __tuple = {0};                                                                         \
    typeof(__key) key__ = (__key);                                                                          \
    char *__data_ptr;                                                                                       \
    memcpy(&__data_ptr, __map_ptr, sizeof(char*));                                                          \
    struct map *__map = (struct map*)__data_ptr;                                                            \
    uint64_t __hash_val = __map->hash_func((void*)&key__);                                                  \
    uint64_t __index = map_find_spot(__map_ptr, (void*)&key__, __hash_val, false);                          \
    __data_ptr += sizeof(struct map) + __index * (__map->key_size + __map->value_size + sizeof(int16_t));   \
    if (!bitset_check(__map->bit_array, __index)) goto __to_return;                                         \
    __tuple.key = __data_ptr;                                                                               \
    if (__map->value_size > 0) __tuple.value = (__data_ptr += __map->key_size, __data_ptr);                 \
    __to_return:                                                                                            \
        __tuple;                                                                                            \
})


#define map_replace(__map_ptr, __key, __new_value) do {                                                         \
    char *__data_ptr;                                                                                           \
    memcpy(&__data_ptr, __map_ptr, sizeof(char*));                                                              \
    struct map *__map = (struct map*)__data_ptr;                                                                \
    typeof(__key) key__ = (__key);                                                                              \
    typeof(__new_value) __new_value__ = (__new_value);                                                          \
    uint64_t __hash_val = __map->hash_func((void*)&key__);                                                      \
    uint64_t __index = map_find_spot(__map_ptr, (void*)&key__, __hash_val, false);                              \
    __data_ptr +=                                                                                               \
        sizeof(struct map) + __index * (__map->key_size + __map->value_size + sizeof(int16_t));                 \
    if (bitset_check(__map->bit_array, __index)) {                                                              \
        if (__map->value_destructor != NULL) {                                                                  \
            if (bitset_check(__map->bit_array, __map->cap + 1)) {                                               \
                char *value1, *value2, *value3 = (char*)&__new_value__;                                         \
                memcpy(&value1, __data_ptr + __map->key_size, sizeof(char*));                                   \
                memcpy(&value2, value3, sizeof(char*));                                                         \
                if (value1 != value2) {                                                                         \
                    __map->value_destructor((void*)value1);                                                     \
                }                                                                                               \
            } else {                                                                                            \
                __map->value_destructor((void*)(__data_ptr + __map->key_size));                                 \
            }                                                                                                   \
        }                                                                                                       \
        struct __attribute__((__packed__)) tuple { typeof(key__) k; typeof(__new_value__) v; int16_t h; };      \
        ((struct tuple*)__data_ptr)[0].v = __new_value__;                                                       \
    }                                                                                                           \
} while (0)



#define map_destroy(__map_ptr) do {                                                                     \
    char *__data_ptr;                                                                                   \
    memcpy(&__data_ptr, __map_ptr, sizeof(char*));                                                      \
    struct map *__map = (struct map*)__data_ptr;                                                        \
    char *__iter = __data_ptr + sizeof(struct map);                                                     \
    uint64_t __offset = __map->key_size + __map->value_size + sizeof(int16_t);                          \
    for (size_t __index = 0UL; __index < __map->cap; ++__index) {                                       \
        int16_t cur_prefix = *(int16_t*)(__iter + __map->key_size + __map->value_size);                 \
        if (!bitset_check(__map->bit_array, __index) || cur_prefix == -1) {                             \
            __iter += __offset;                                                                         \
            continue;                                                                                   \
        }                                                                                               \
        if (__map->key_destructor != NULL) {                                                            \
            if (bitset_check(__map->bit_array, __map->cap)) {                                           \
                char *key;                                                                              \
                memcpy(&key, __iter, sizeof(char*));                                                    \
                __map->key_destructor((void*)key);                                                      \
            } else {                                                                                    \
                __map->key_destructor((void*)(__iter));                                                 \
            }                                                                                           \
        }                                                                                               \
        if (__map->value_destructor != NULL) {                                                          \
            if (bitset_check(__map->bit_array, __map->cap + 1)) {                                       \
                char *value;                                                                            \
                memcpy(&value, __iter + __map->key_size, sizeof(char*));                                \
                __map->value_destructor((void*)value);                                                  \
            } else {                                                                                    \
                __map->value_destructor((void*)(__iter + __map->key_size));                             \
            }                                                                                           \
        }                                                                                               \
        __iter += __offset;                                                                             \
    }                                                                                                   \
    bitset_destroy(__map->bit_array);                                                                   \
    free(__data_ptr);                                                                                   \
} while (0)


#endif /* MAP_H */