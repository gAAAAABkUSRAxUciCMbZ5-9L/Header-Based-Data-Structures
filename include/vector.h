#ifndef VECTOR_H
#define VECTOR_H


#ifdef __cplusplus
extern "C" {
#endif
    #ifndef _POSIX_C_SOURCE
        #define _POSIX_C_SOURCE 200809L
    #endif

	#ifndef _GNU_SOURCE
        #define _GNU_SOURCE
    #endif
	#include <stdio.h>
	#include <string.h>
	#include <stdlib.h>
	#include <stdbool.h>
	#include <stdint.h>
#ifdef __cplusplus
}
#endif



#define VEC_MIN_CAP 32


struct vector {
	uint64_t size;
	uint64_t cap;
	uint64_t entry_size;
	bool     is_ptr;
	void     (*destructor)(void *v);
};


#define vector_factor(__size, __cap) ({		    \
	uint64_t __mult = 1;					    \
	while ((__size) > ((__cap) << __mult++))	\
		;									    \
	((__cap) << __mult);						\
})


#define vector_set_size(__vec, __size) do {								\
	void *__data_ptr = (void*)(__vec);									\
	((&((struct vector*)(*(char**)__data_ptr))[-1])->size = (__size));	\
} while (0)


#define vector_set_cap(__vec, __cap) do {										\
	void *__data_ptr = (void*)(__vec);											\
	((&((struct vector*)(*(char**)__data_ptr))[-1])->cap = (__cap));			\
} while (0)


#define vector_set_flag(__vec, __flag) do {											\
	void *__data_ptr = (void*)(__vec);												\
	((&((struct vector*)(*(char**)__data_ptr))[-1])->is_ptr = (__flag));			\
} while (0)


#define vector_set_destructor(__vec, __destructor) do {								\
	void *__data_ptr = (void*)(__vec);												\
	((&((struct vector*)(*(char**)__data_ptr))[-1])->destructor = (__destructor));	\
} while (0)


#define vector_set_entry_size(__vec, __entry_size) do {								\
	void *__data_ptr = (void*)(__vec);												\
	((&((struct vector*)(*(char**)__data_ptr))[-1])->entry_size = (__entry_size));	\
} while (0)


#define vector_get_size(__vec) ({								\
	void *__data_ptr = (void*)(__vec);							\
	(&((struct vector*)(*(char**)__data_ptr))[-1])->size;		\
})


#define vector_get_cap(__vec) ({								\
	void *__data_ptr = (void*)(__vec);							\
	(&((struct vector*)(*(char**)__data_ptr))[-1])->cap;		\
})


#define vector_get_flag(__vec) ({								\
	void *__data_ptr = (void*)(__vec);							\
	(&((struct vector*)(*(char**)__data_ptr))[-1])->is_ptr;		\
})


#define vector_get_destructor(__vec) ({							\
	void *__data_ptr = (void*)(__vec);							\
	(&((struct vector*)(*(char**)__data_ptr))[-1])->destructor;	\
})


#define vector_get_entry_size(__vec) ({							\
	void *__data_ptr = (void*)(__vec);							\
	(&((struct vector*)(*(char**)__data_ptr))[-1])->entry_size;	\
})



#define vector_create(__type, __init_cap, __destructor, __ptr_flag) ({			\
	uint64_t bytes = sizeof(struct vector) + (__init_cap) * sizeof(__type);		\
	char *__vec = (char*)calloc(1, bytes) + sizeof(struct vector);				\
	vector_set_size(&__vec, 0);													\
	vector_set_cap(&__vec, (__init_cap));										\
	vector_set_destructor(&__vec, (__destructor));								\
	vector_set_entry_size(&__vec, sizeof(__type));								\
	vector_set_flag(&__vec, (__ptr_flag));										\
	((__type*)__vec);															\
})


#define vector_resize(__vec_ptr, __new_vec_cap) do {									\
	uint64_t __vec_entry_size = vector_get_entry_size(__vec_ptr);						\
	uint64_t __new_size = sizeof(struct vector) + (__new_vec_cap) * __vec_entry_size;	\
	void *__base_ptr = (void*)(__vec_ptr);												\
	char *__new_ptr = (char*)calloc(1, __new_size);										\
	memcpy(__new_ptr, ((char*)(*(char**)__base_ptr) - sizeof(struct vector)),			\
           sizeof(struct vector) + __vec_entry_size * (vector_get_size(__vec_ptr)));	\
	__new_ptr += sizeof(struct vector);													\
	char *to_free = (char*)(*(char**)__base_ptr) - sizeof(struct vector);				\
	memcpy(__base_ptr, &__new_ptr, sizeof(char*));										\
	free(to_free);																		\
	vector_set_cap(__vec_ptr, __new_vec_cap);											\
} while (0)


#define vector_insert(__vec_ptr, __entry) do {													\
	uint64_t __vec_size       = vector_get_size(__vec_ptr);										\
	uint64_t __vec_cap        = vector_get_cap(__vec_ptr);										\
	typeof((__entry)) entry__ = (__entry);														\
	if (__vec_size == __vec_cap)																\
		vector_resize(__vec_ptr, (__vec_cap << 1));												\
	void *__data_ptr = (void*)(__vec_ptr);														\
	(*(typeof(entry__)**)__data_ptr)[__vec_size] = (entry__);									\
	vector_set_size(__vec_ptr, __vec_size + 1);													\
} while (0)


#define vector_destroy(__vec_ptr) do {																		\
	void *__data_ptr = (void*)(__vec_ptr);																	\
	void (*__vec_destructor)(void *v) = vector_get_destructor(__vec_ptr);									\
	if (__vec_destructor) {																					\
		uint64_t __vec_entry_size = vector_get_entry_size(__vec_ptr);										\
		bool __vec_is_ptr = vector_get_flag(__vec_ptr);														\
		for (uint64_t __index = 0UL; __index < vector_get_size((__vec_ptr)); ++__index) {					\
			if (__vec_is_ptr) {																				\
				char *__entry;																				\
				memcpy(&__entry, (*((char**)__data_ptr)) + __index * __vec_entry_size, __vec_entry_size);	\
				__vec_destructor((void*)__entry);															\
			} else {																						\
				__vec_destructor((void*)((*(char**)__data_ptr) + __index * __vec_entry_size));				\
			}																								\
		}																									\
	}																										\
	free((*(char**)__data_ptr) - sizeof(struct vector));													\
} while (0)


#define vector_delete(__vec_ptr, __index) do {														\
	if (0 <= (__index) && (__index) < (vector_get_size(__vec_ptr))) {								\
		void *__data_ptr = (void*)(__vec_ptr);														\
		void (*__vec_destructor)(void *v) = vector_get_destructor(__vec_ptr);						\
		uint64_t __vec_size = vector_get_size(__vec_ptr);											\
		uint64_t __vec_entry_size = vector_get_entry_size(__vec_ptr);								\
		uint64_t index__ = (__index);																\
		if (__vec_destructor) {																		\
			bool __vec_is_ptr = vector_get_flag(__vec_ptr);											\
			if (__vec_is_ptr) {																		\
				char *__entry;																		\
				memcpy(&__entry, *(char**)__data_ptr + index__ * __vec_entry_size, sizeof(char*));	\
				__vec_destructor((void*)__entry);													\
			} else {																				\
				__vec_destructor((void*)(*(char**)__data_ptr + index__ * __vec_entry_size));		\
			}																						\
		}																							\
		memmove(((*(char**)__data_ptr)) + (index__) * __vec_entry_size,								\
	            ((*(char**)__data_ptr)) + (index__ + 1) * __vec_entry_size,							\
			    __vec_entry_size * (__vec_size - 1 - (index__)));									\
		vector_set_size(__vec_ptr, __vec_size - 1);													\
	}																								\
} while (0)


#define vector_find(__vec_ptr, __entry, __cmp_func) ({												\
	void *__data_ptr = (void*)(__vec_ptr);															\
	uint64_t __vec_entry_size = vector_get_entry_size(__vec_ptr);									\
	uint64_t __vec_size = vector_get_size(__vec_ptr);												\
	size_t __index;																					\
	bool __found = false;																			\
	typeof(__entry) __to_search = (__entry);														\
	int (*cmp_func__)(void *a, void *b) = (__cmp_func);												\
	char *entry__ = (char*)(*((char**)__data_ptr));													\
	for (__index = 0UL; __index < __vec_size && !__found; ++__index) {								\
		if (cmp_func__(entry__ + __index * __vec_entry_size, &__to_search) == 0) __found = true;	\
	}																								\
	(!__found ? -1 : __index - 1);																	\
})


#define vector_append(__vec_ptr, __array, __size) do {					\
	uint64_t __vec_size       = vector_get_size(__vec_ptr);				\
	uint64_t __vec_cap        = vector_get_cap(__vec_ptr);				\
	uint64_t __vec_entry_size = vector_get_entry_size(__vec_ptr);		\
	uint64_t __space_left     = __vec_cap - __vec_size;					\
	if (__space_left < (__size))										\
		vector_resize(__vec_ptr, vector_factor((__size), __vec_cap));	\
	void *__data_ptr = (void*)(__vec_ptr);								\
	memcpy((*(char**)__data_ptr) + __vec_entry_size * __vec_size,		\
	      (__array), (__size) * __vec_entry_size);						\
	vector_set_size(__vec_ptr, __vec_size + (__size));					\
} while (0)


#define vector_reset(__vec_ptr, __reset_cap) do {														\
	void (*__vec_destructor)(void *v) = vector_get_destructor(__vec_ptr);								\
	bool __vec_is_ptr = vector_get_flag(__vec_ptr);														\
	void *__data_ptr = (void*)(__vec_ptr);																\
	if (__vec_destructor != NULL) {																		\
		uint64_t __vec_entry_size = vector_get_entry_size(__vec_ptr);									\
		for (size_t __index = 0UL; __index < vector_get_size(__vec_ptr); ++__index) {					\
			if (__vec_is_ptr) {																			\
				char *__entry;																			\
				memcpy(&__entry, (*(char**)__data_ptr) + __index * __vec_entry_size, sizeof(char*));	\
				__vec_destructor((void*)__entry);														\
			} else {																					\
				__vec_destructor((void*)((*(char**)__data_ptr) + __index * __vec_entry_size));			\
			}																							\
		}																								\
	}																									\
	char *__base_ptr = (char*)(__vec_ptr);																\
	void *__data_ptr = (void*)(__vec_ptr);																\
	uint64_t __vec_entry_size = vector_get_entry_size(__vec_ptr);										\
	char *__new_ptr = calloc(1, (__reset_cap) * __vec_entry_size + sizeof(struct vector));				\
	char *__to_free = (char*)((char*)(*(char**)__data_ptr) - sizeof(struct vector));					\
	__new_ptr += sizeof(struct vector);																	\
	memcpy(__base_ptr, &__new_ptr, sizeof(char*));														\
	vector_set_cap(__vec_ptr, (__reset_cap));															\
	vector_set_size(__vec_ptr, 0);																		\
	free(__to_free);																					\
} while (0)


#define vector_delete_all(__vec_ptr, __entry, __cmp_func) do {								\
	void *__base_ptr = (void*)(__vec_ptr);													\
	uint64_t __index = 0;																	\
	typeof(__entry) entry__ = (__entry);													\
	uint64_t __vec_entry_size = vector_get_entry_size((__vec_ptr));							\
	int (*cmp_func__)(void *a, void *b) = (__cmp_func);										\
	while (__index < vector_get_size(__vec_ptr)) {											\
		int res = cmp_func__(*((char**)__base_ptr) + __index * __vec_entry_size, &entry__);	\
		if (res == 0) {																		\
			vector_delete(__vec_ptr, __index);												\
		} else {																			\
			__index++;																		\
		}																					\
	}																						\
} while (0)


#define vector_insert_at(__vec_ptr, __index, __entry) do {						\
	typeof(__entry) entry__ = (__entry);										\
	uint64_t index__ = (__index);												\
	uint64_t __vec_cap  = vector_get_cap(__vec_ptr);							\
	uint64_t __vec_size = vector_get_size(__vec_ptr);							\
	uint64_t __vec_entry_size = vector_get_entry_size(__vec_ptr);				\
	if (0 <= (index__) && (index__) <= __vec_size) {							\
		if (__vec_cap == __vec_size)											\
			vector_resize(__vec_ptr, __vec_cap << 1);							\
		void *__data_ptr = (void*)(__vec_ptr);									\
		if ((index__) < __vec_size) {											\
			memmove(&(*(typeof(entry__)**)__data_ptr)[index__ + 1],				\
		            &(*(typeof(entry__)**)__data_ptr)[index__],					\
				    __vec_entry_size * (__vec_size - index__));					\
		}																		\
		(*(typeof(entry__)**)__data_ptr)[index__] = (entry__);					\
		vector_set_size(__vec_ptr, __vec_size + 1);								\
	}																			\
} while (0)

#endif /* VECTOR_H */
