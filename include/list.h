#ifndef DLL_LIST_H
#define DLL_LIST_H

#ifdef __cplusplus
extern "C" {
#endif
    #ifndef _POSIX_C_SOURCE
        #define _POSIX_C_SOURCE 200809L
    #endif

    #ifndef _GNU_SOURCE
        #define _GNU_SOURCE
    #endif
    #include <stdbool.h>
    #include <stdlib.h>
    #include <stdint.h>
    #include <string.h>
    #include <stdio.h>

#ifdef __cplusplus
}
#endif

struct dll_list {
    struct dll_node *first;
    struct dll_node *last;
    uint64_t         size;
    uint64_t         entry_size;
    void            (*destructor)(void *v);
    bool             is_ptr;
};

struct dll_node {
    struct dll_node *prev;
    struct dll_node *next;
    char *entry;
};


#define dll_create(__type, __destructor, __is_ptr) ({                               \
    struct dll_list *__dll = (struct dll_list*)calloc(1, sizeof(struct dll_list));  \
    __dll->entry_size = sizeof(__type);                                             \
    __dll->destructor = (__destructor);                                             \
    __dll->is_ptr = (__is_ptr);                                                     \
    __dll;                                                                          \
})


#define dll_insert_end(__dll_ptr, __entry) do {                                     \
    typeof(__entry) entry__ = (__entry);                                            \
    struct dll_node *__new = (struct dll_node*)calloc(1, sizeof(struct dll_node));  \
    __new->entry = (char*)malloc((__dll_ptr)->entry_size);                          \
    memcpy(__new->entry, &entry__, (__dll_ptr)->entry_size);                        \
    if (!(__dll_ptr)->size) {                                                       \
        (__dll_ptr)->first = (__dll_ptr)->last = __new;                             \
    } else {                                                                        \
        (__dll_ptr)->last->next = __new;                                            \
        __new->prev = (__dll_ptr)->last;                                            \
        (__dll_ptr)->last = __new;                                                  \
    }                                                                               \
    (__dll_ptr)->size++;                                                            \
} while (0)


#define dll_insert_start(__dll_ptr, __entry) do {                                   \
    typeof(__entry) entry__ = (__entry);                                            \
    struct dll_node *__new = (struct dll_node*)calloc(1, sizeof(struct dll_node));  \
    __new->entry = (char*)malloc((__dll_ptr)->entry_size);                          \
    memcpy(__new->entry, &entry__, sizeof(entry__));                                \
    if (__dll_ptr->size == 0) {                                                     \
        __dll_ptr->last = __new;                                                    \
    } else {                                                                        \
        __dll_ptr->first->prev = __new;                                             \
        __new->next = __dll_ptr->first;                                             \
    }                                                                               \
    __dll_ptr->first = __new;                                                       \
    (__dll_ptr)->size++;                                                            \
} while (0)


#define dll_insert_after(__dll_ptr, __dll_node, __entry) do {                               \
    if (__dll_ptr->size != 0) {                                                             \
        struct dll_node *__node = (__dll_node);                                             \
        if (__node->next == NULL) {                                                         \
            dll_insert_end(__dll_ptr, __entry);                                             \
        } else {                                                                            \
            struct dll_node *__node = (__dll_node);                                         \
            typeof(__entry) entry__ = (__entry);                                            \
            __dll_ptr->size++;                                                              \
            struct dll_node *__new = (struct dll_node*)calloc(1, sizeof(struct dll_node));  \
            __new->entry = (char*)malloc(__dll_ptr->entry_size);                            \
            memcpy(__new->entry, &entry__, __dll_ptr->entry_size);                          \
            struct dll_node *__temp = __node->next;                                         \
            __node->next = __new;                                                           \
            __new->prev = __node;                                                           \
            __new->next = __temp;                                                           \
            __temp->prev = __new;                                                           \
        }                                                                                   \
    }                                                                                       \
} while (0)


#define dll_insert_before(__dll_ptr, __dll_node, __entry) do {                          \
    if (__dll_ptr->size != 0) {                                                         \
        struct dll_node *__node = (__dll_node);                                         \
        if (__node->prev == NULL) {                                                     \
            dll_insert_start(__dll_ptr, __entry);                                       \
        } else {                                                                        \
            typeof(__entry) entry__ = (__entry);                                        \
            struct dll_node *__node = (__dll_node);                                     \
            __dll_ptr->size++;                                                          \
            struct dll_node *__new = (struct dll_node*)malloc(sizeof(struct dll_node)); \
            __new->entry = (char*)malloc(__dll_ptr->entry_size);                        \
            memcpy(__new->entry, &entry__, __dll_ptr->entry_size);                      \
            struct dll_node *__temp = __node->prev;                                     \
            __temp->next = __new;                                                       \
            __new->prev = __temp;                                                       \
            __new->next = __node;                                                       \
            __node->prev = __new;                                                       \
        }                                                                               \
    }                                                                                   \
} while (0)


#define dll_delete(__dll_ptr, __dll_node) do {                              \
    struct dll_node *__node = (__dll_node);                                 \
    struct dll_list *__list = (__dll_ptr);                                  \
    if (__list->size != 0) {                                                \
        if (__node->prev == NULL) {                                         \
    		__list->first = __node->next;                                   \
        } else {                                                            \
            __node->prev->next = __node->next;                              \
        }                                                                   \
        if (__node->next == NULL) {                                         \
            __list->last = __node->prev;                                    \
        } else {                                                            \
            __node->next->prev = __node->prev;                              \
        }                                                                   \
        if (__list->destructor != NULL) {                                   \
            if (__list->is_ptr) {                                           \
                char *__ptr;                                                \
                memcpy(&__ptr, __node->entry, sizeof(char*));               \
                __list->destructor((void*)__ptr);                           \
            } else {                                                        \
                __list->destructor((void*)__node->entry);                   \
            }                                                               \
        }                                                                   \
        __list->size--;                                                     \
        free(__node->entry);                                                \
        free(__node);                                                       \
    }                                                                       \
} while (0)


#define dll_find(__dll_ptr, __entry, __cmp_fun) ({                      \
    __label__ __to_return;                                              \
    typeof(__entry) entry__ = (__entry);                                \
    struct dll_node *__iter = (__dll_ptr)->first;                       \
    while (__iter != NULL) {                                            \
        if (__cmp_fun(&entry__, __iter->entry) == 0) goto __to_return;  \
        __iter = __iter->next;                                          \
    }                                                                   \
    __to_return:                                                        \
        __iter;                                                         \
})


#define dll_print(__dll_ptr, __print_func, __is_reverse) do {                       \
    struct dll_node *__iter = __is_reverse ? __dll_ptr->last : __dll_ptr->first;    \
    while (__iter != NULL) {                                                        \
        __print_func((void*)__iter->entry);                                         \
        __iter = __is_reverse ? __iter->prev : __iter->next;                        \
    }                                                                               \
} while (0)


#define dll_destroy(__dll_ptr) do {                             \
    struct dll_node *__iter = (__dll_ptr)->first;               \
    while (__iter != NULL) {                                    \
        struct dll_node *__temp = __iter;                       \
        __iter = __iter->next;                                  \
        if ((__dll_ptr)->destructor != NULL) {                  \
            if ((__dll_ptr->is_ptr)) {                          \
                char *__ptr;                                    \
                memcpy(&__ptr, __temp->entry, sizeof(char*));   \
                (__dll_ptr)->destructor((void*)__ptr);          \
            } else {                                            \
                (__dll_ptr)->destructor((void*)__temp->entry);  \
            }                                                   \
        }                                                       \
        free(__temp->entry);                                    \
        free(__temp);                                           \
    }                                                           \
    free(__dll_ptr);                                            \
} while (0)



#endif /* DLL_LIST_H */