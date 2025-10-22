#include "vector.h"
#include "acutest.h"


#define ARRAY_SIZE(arr) (sizeof((arr)) / sizeof((arr)[0]))



struct person {
    char *name;
    char *surname;
    int   id;
};


static void person_destructor(void *v) {
    free(((struct person*)v)->name);
    free(((struct person*)v)->surname);
}


static int *create_int(int val) {
    int *ptr = malloc(sizeof(int));
    *ptr = val;
    return ptr;
}


static struct person *create_persons(uint32_t size) {
    char *names[] = {
        "Alice", "Benjamin", "Charlotte", "David", "Ella",
        "Finn", "Grace", "Henry", "Isla", "Jack",
        "Katherine", "Liam", "Mia", "Noah", "Olivia",
        "Peter", "Quinn", "Ruby", "Samuel", "Tessa"
    };

    char *surnames[] = {
        "Anderson", "Brown", "Carter", "Davis", "Evans",
        "Foster", "Garcia", "Hughes", "Irwin", "Jackson",
        "Kennedy", "Lee", "Miller", "Nelson", "Owens",
        "Parker", "Quincy", "Reed", "Stewart", "Turner"
    };

    struct person *persons = malloc(size * sizeof(struct person));
    for (int i = 0; i < size; ++i)
        persons[i] = (struct person){
            .id      = i,
            .name    = strdup(names[i % ARRAY_SIZE(names)]),
            .surname = strdup(surnames[i % ARRAY_SIZE(surnames)]) 
        };
    return persons;
}


static int cmp_ints(void *a, void *b) {
    int __a = *(int*)a;
    int __b = *(int*)b;
    return (__a > __b) - (__a < __b);
}


static int cmp_person_id(void *a, void *b) {
    return cmp_ints(&((struct person*)a)->id,
                    &((struct person*)b)->id);
}


static int cmp_person_name(void *a, void *b) {
    return strcmp(((struct person*)a)->name,
                  ((struct person*)b)->name);
}


static int cmp_person_surname(void *a, void *b) {
    return strcmp(((struct person*)a)->surname,
                  ((struct person*)b)->surname);
}


static int cmp_person(void *a, void *b) {
    int val1 = cmp_person_id(a, b);
    int val2 = cmp_person_name(a, b);
    int val3 = cmp_person_surname(a, b);
    return !(!val1 && !val2 && !val3);
}



void test_vector_create(void) {
    int N = 32;
    int **int_vec = vector_create(int*, N, free, true);
    struct person *person_vec = vector_create(struct person, 2 * N,
                                              person_destructor, false);

    TEST_ASSERT(int_vec != NULL && person_vec != NULL);
    TEST_ASSERT(vector_get_cap(&int_vec) == N);
    TEST_ASSERT(vector_get_cap(&person_vec) == 2 * N);
    TEST_ASSERT(vector_get_flag(&int_vec));
    TEST_ASSERT(!vector_get_flag(&person_vec));
    TEST_ASSERT(vector_get_size(&int_vec) == 0);
    TEST_ASSERT(vector_get_size(&person_vec) == 0);
    TEST_ASSERT(vector_get_destructor(&int_vec) == free);
    TEST_ASSERT(vector_get_destructor(&person_vec) == person_destructor);

    vector_destroy(&int_vec);
    vector_destroy(&person_vec);
}


void test_vector_insert(void) {
    int N = 100;
    int *vec_ptr = vector_create(int, VEC_MIN_CAP, NULL, false);

    srand(time(NULL));

    for (int i = 0UL; i < N; ++i) {
        vector_insert(&vec_ptr, i);
        TEST_ASSERT(vec_ptr[i] == i);
        TEST_ASSERT(vector_get_size(&vec_ptr) == i + 1);
    }
    TEST_ASSERT(vector_get_size(&vec_ptr) == N);

    for (int i = 0UL; i < vector_get_size(&vec_ptr); ++i)
        TEST_ASSERT(vec_ptr[i] == i);

    int old_sz = vector_get_size(&vec_ptr);
    int index = rand() % (vector_get_size(&vec_ptr) - 2) + 1;
    TEST_ASSERT(index >= 0 && index < vector_get_size(&vec_ptr));
    vector_insert_at(&vec_ptr, index, N);
    TEST_ASSERT(vector_get_size(&vec_ptr) == old_sz + 1);

    for (int i = 0, j = 0; i < vector_get_size(&vec_ptr); ++i)
        vec_ptr[i] == (i == index ? N : j++);

    vector_destroy(&vec_ptr);
}



void test_vector_delete(void) {
    int N = 1000;
    int *vec_ptr = vector_create(int, VEC_MIN_CAP, NULL, false);

    srand(time(NULL));
    for (int i = 0; i < N; ++i)
        vector_insert(&vec_ptr, i);

    size_t index = rand() % (vector_get_size(&vec_ptr) - 1);
    uint64_t old_sz = vector_get_size(&vec_ptr);
    vector_delete(&vec_ptr, index);
    
    TEST_ASSERT(vector_get_size(&vec_ptr) == old_sz - 1);
    for (int i = 0; i < vector_get_size(&vec_ptr); ++i)
        TEST_ASSERT(vec_ptr[i] == (i + (i >= index)));

    for (int i = vector_get_size(&vec_ptr) - 1; i >= 0; i--) {
        vector_delete(&vec_ptr, i);
        TEST_ASSERT(vector_get_size(&vec_ptr) == i);
    }

    for (int i = 0; i < N; ++i)
        vector_insert(&vec_ptr, (int){rand() % 3 == 0 ? 1 : i + 2});

    vector_delete_all(&vec_ptr, (int){1}, cmp_ints);
    for (int i = 0; i < vector_get_size(&vec_ptr); ++i)
        TEST_ASSERT(vec_ptr[i] != 1);

    vector_destroy(&vec_ptr);
}


void test_vector_combine(void) {
    int N = 2000;
    int *vec1 = vector_create(int, N, NULL, false);
    int *vec2 = vector_create(int, N, NULL, false);

    for (int i = 0; i < N; ++i) {
        vector_insert(&vec1, i);
        vector_insert(&vec2, i + N);
    }
    vector_append(&vec1, vec2, vector_get_size(&vec2));

    TEST_ASSERT(vector_get_size(&vec1) == 2 * N);
    for (int i = 0; i < vector_get_size(&vec1); ++i)
        TEST_ASSERT(vec1[i] == i);

    vector_destroy(&vec1);
    vector_destroy(&vec2);

    struct person *persons = create_persons(N);

    struct person *vec3 = vector_create(struct person, VEC_MIN_CAP, 
                                        person_destructor, false);
    
    for (int i = 0; i < N; ++i) {
        vector_insert(&vec3, persons[i]);
        int index = vector_find(&vec3, persons[i], cmp_person);
        TEST_ASSERT(cmp_person(&vec3[index], &persons[i]) == 0);
    }

    TEST_ASSERT(vector_find(&vec3, ((struct person){.id = N}), cmp_person_id) == -1);
    TEST_ASSERT(vector_find(&vec3, ((struct person){.name = "Alexandros"}), cmp_person_name) == -1);
    TEST_ASSERT(vector_find(&vec3, ((struct person){.surname = "Misas"}), cmp_person_surname) == -1);


    int index = vector_find(&vec3, ((struct person){.id = 12}), cmp_person_id);
    TEST_ASSERT(vec3[index].id == 12);

    index = vector_find(&vec3, ((struct person){.name = "Noah"}), cmp_person_name);
    TEST_ASSERT(strcmp(vec3[index].name, "Noah") == 0);

    index = vector_find(&vec3, ((struct person){.surname = "Jackson"}), cmp_person_surname);
    TEST_ASSERT(strcmp(vec3[index].surname, "Jackson") == 0);

    vector_delete(&vec3, vector_find(&vec3, ((struct person){.id = 15}), cmp_person_id));
    TEST_ASSERT(vector_find(&vec3, ((struct person){.id = 15}), cmp_person_id) == -1);

    char *name    = strdup(vec3[0].name);
    char *surname = strdup(vec3[vector_get_size(&vec3) - 1].surname);

    for (int i = 0; i < 5; ++i) {
        struct person temp = { 
            .name = strdup(name), 
            .surname = strdup("foo")
        };
        vector_insert_at(&vec3, i + 2, temp);
    }

    for (int i = 5; i < 10; ++i) {
        struct person temp = {
            .name = strdup("foo"),
            .surname = strdup(surname)
        };
        vector_insert_at(&vec3, i + 2, temp);
    }


    vector_delete_all(&vec3, ((struct person){.name = name}), cmp_person_name);
    TEST_ASSERT(vector_find(&vec3, ((struct person){.name = name}), cmp_person_name) == -1);

    vector_delete_all(&vec3, ((struct person){.surname = surname}), cmp_person_surname);
    TEST_ASSERT(vector_find(&vec3, ((struct person){.surname = surname}), cmp_person_surname) == -1);

    for (int i = 0; i < vector_get_size(&vec3); ++i) {
        TEST_ASSERT(strcmp(vec3[i].name, name));
        TEST_ASSERT(strcmp(vec3[i].surname, surname));
    }
    vector_destroy(&vec3);
    free(persons);
    free(surname);
    free(name);
}





TEST_LIST = {
    "test_vector_create" , test_vector_create,
    "test_vector_insert" , test_vector_insert,
    "test_vector_delete" , test_vector_delete,
    "test_vector_combine", test_vector_combine,
    NULL,                  NULL
};
