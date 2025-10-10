#ifndef STACK_H
#define STACK_H

#include "vector.h"

#define STACK_MIN_CAP VEC_MIN_CAP

#define stack_create(__entry_sz, __cap, __stack_destructor, __is_ptr)  \
    vector_create(__entry_sz, __cap, __stack_destructor, __is_ptr)


#define stack_get_size(__stack_ptr) \
    vector_get_size(__stack_ptr)

#define stack_get_size(__stack_ptr) \
    vector_get_size(__stack_ptr)


#define stack_get_cap(__stack_ptr) \
    vector_get_cap(__stack_ptr)


#define stack_get_flag(__stack_ptr) \
    vector_get_flag(__stack_ptr)


#define stack_get_destructor(__stack_ptr) \
    vector_get_destructor(__stack_ptr)


#define stack_set_size(__stack_ptr, __size) \
    vector_set_size(__stack_ptr, __size)


#define stack_set_flag(__stack_ptr, __flag) \
    vector_set_flag(__stack_ptr, __flag)


#define stack_set_destructor(__stack_ptr, __destr) \
    vector_set_destructor(__stack_ptr, __destr)


#define stack_insert_top(__stack_ptr, __entry)  \
    vector_insert(__stack_ptr, __entry)


#define stack_remove_top(__stack_ptr)  \
    vector_delete(__stack_ptr, (vector_get_size(__stack_ptr)) - 1)

    
#define stack_destroy(__stack_ptr)  \
    vector_destroy(__stack_ptr)



#endif /* STACK_H */