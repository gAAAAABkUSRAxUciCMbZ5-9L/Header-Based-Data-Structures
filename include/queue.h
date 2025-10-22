#ifndef QUEUE_H
#define QUEUE_H

#include "vector.h"

#define QUEUE_MIN_CAP VEC_MIN_CAP

#define queue_create(__entry_sz, __cap, __queue_destructor, __is_ptr)  \
    vector_create(__entry_sz, __cap, __queue_destructor, __is_ptr)


#define queue_get_size(__queue_ptr) \
    vector_get_size(__queue_ptr)


#define queue_get_cap(__queue_ptr) \
    vector_get_cap(__queue_ptr)


#define queue_get_flag(__queue_ptr) \
    vector_get_flag(__queue_ptr)


#define queue_get_destructor(__queue_ptr) \
    vector_get_destructor(__queue_ptr)


#define queue_set_size(__queue_ptr, __size) \
    vector_set_size(__queue_ptr, __size)


#define queue_set_flag(__queue_ptr, __flag) \
    vector_set_flag(__queue_ptr, __flag)


#define queue_set_destructor(__queue_ptr, __destr) \
    vector_set_destructor(__queue_ptr, __destr)


#define queue_insert_back(__queue_ptr, __entry) \
    vector_insert(__queue_ptr, __entry)


#define queue_get_front(__queue_ptr) \
    ((__queue_ptr)[0])


#define queue_remove_front(__queue_ptr) \
    vector_delete(__queue_ptr, 0)

    
#define queue_destroy(__queue_ptr)  \
    vector_destroy(__queue_ptr)



#endif /* QUEUE_H */