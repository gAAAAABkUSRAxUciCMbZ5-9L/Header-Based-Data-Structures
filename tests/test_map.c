#include "map.h"
#include "acutest.h"

#include <sys/time.h>


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


static int cmp_strings(void *a, void *b) {
    return strcmp(*(char**)a, *(char**)b);
}



static int cmp_ints(void *a, void *b) {
    int aa = *(int*)a;
    int bb = *(int*)b;
    return (aa > bb) - (aa < bb);
}


static int cmp_point_ints(void *a, void *b) {
    int *aa = *(int**)a;
    int *bb = *(int**)b;
    return cmp_ints(aa, bb);
}


static uint64_t hash_ints(void *v) {
    return *(int*)v;
}


static uint64_t hash_strings(void *v) {
    uint64_t hash = 5381;
    char *str = *(char**)v;

    int c;
    while ((c = *str++))
        hash = ((hash << 5) + hash) + c;

    return hash;
}

static uint64_t hash_person_id(void *v) {
    return hash_ints(&((struct person*)v)->id);
}

static int cmp_person_id(void *a, void *b) {
    return cmp_ints(&((struct person*)a)->id,
                    &((struct person*)b)->id);
}

static bool eq_persons(struct person *a, struct person *b) {
    return a->id == b->id 
        && strcmp(a->name, b->name) == 0
        && strcmp(a->surname, b->surname) == 0;
}


static uint64_t hash_point_ints(void *v) {
    return hash_ints(*(int**)v);
}


static int *create_int(int val) {
    int *ptr = malloc(sizeof(int));
    *ptr = val;
    return ptr;
}

static void shuffle(int **array, size_t n) {    
    struct timeval tv;
    gettimeofday(&tv, NULL);
    int usec = tv.tv_usec;
    srand48(usec);


    if (n > 1) {
        size_t i;
        for (i = n - 1; i > 0; i--) {
            size_t j = (unsigned int) (drand48()*(i+1));
            int *t = array[j];
            array[j] = array[i];
            array[i] = t;
        }
    }
}


static void insert_and_test(struct map **map, void *key, void *value, bool new_insert) {
    uint64_t old_sz = (*map)->size;
    map_insert(map, key, value);
    struct map_tuple t = map_find(map, key);
    TEST_ASSERT(t.key != NULL && t.value != NULL);
    TEST_ASSERT(*(int**)t.key == (int*)key);
    TEST_ASSERT(*(int**)t.value == (int*)value);
    TEST_ASSERT(**(int**)t.key == *(int*)key);
    TEST_ASSERT(**(int**)t.value == *(int*)value);
    TEST_ASSERT((*map)->size == old_sz + (new_insert));
}

static void remove_and_test(struct map **map, void *key, bool exists) {
    uint64_t old_sz = (*map)->size ;
    void *dupl_key = malloc((*map)->key_size);
    memcpy(dupl_key, key, (*map)->key_size);
    map_delete(map, key);
    struct map_tuple t = map_find(map, dupl_key);
    TEST_ASSERT(t.key == NULL && t.value == NULL);
    TEST_ASSERT((*map)->size == (old_sz - (exists)));
    free(dupl_key);
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


void test_map_create(void) {
    struct map *map = map_create(sizeof(int), sizeof(struct person*), false, true,
                                 MAP_MIN_CAP, hash_ints, cmp_ints, NULL, person_destructor);
    TEST_ASSERT(map != NULL);
    TEST_ASSERT(map->size == 0);
    TEST_ASSERT(map->cap == MAP_MIN_CAP);
    TEST_ASSERT(map->hash_func == hash_ints);
    TEST_ASSERT(map->cmp_keys == cmp_ints);
    TEST_ASSERT(map->key_destructor == NULL);
    TEST_ASSERT(map->value_destructor == person_destructor);
    TEST_ASSERT(map->key_size == sizeof(int));
    TEST_ASSERT(map->value_size == sizeof(struct person*));
    TEST_ASSERT(map->bit_array != NULL);
    map_destroy(&map);
}


void test_map_insert(void) {
    int N = 10000;
    struct map *map = map_create(sizeof(int*), sizeof(int*), true, 
                                 true, MAP_MIN_CAP, hash_point_ints,
                                 cmp_point_ints, free, free);

    int **key_arr = malloc(N * sizeof(int*));
    int **val_arr = malloc(N * sizeof(int*));
    for (int i = 0; i < N; ++i)
        key_arr[i] = create_int(i);

        
    shuffle(key_arr, N);
    for (int i = 0; i < N; ++i) {
        val_arr[i] = create_int(i);
        insert_and_test(&map, key_arr[i], val_arr[i], true);
    }
    TEST_ASSERT(map->size == N);

    int val = *key_arr[0];
    int *new_key = create_int(val);
    int *new_val = create_int(val);
    insert_and_test(&map, new_key, new_val, false);


    insert_and_test(&map, new_key, new_val, false);

    new_key = create_int(val);
    insert_and_test(&map, new_key, new_val, false);

    new_val = create_int(val);
    insert_and_test(&map, new_key, new_val, false);

    insert_and_test(&map, new_key, new_val, false);

    new_val = create_int(val);
    map_replace(&map, new_key, new_val);
    insert_and_test(&map, new_key, new_val, false);

    map_replace(&map, new_key, new_val);
    insert_and_test(&map, new_key, new_val, false);
    
    map_destroy(&map);

    map = map_create(sizeof(int*), sizeof(int*), true, 
                    true, MAP_MIN_CAP, hash_point_ints,
                    cmp_point_ints, NULL, NULL);

    int key1 = 0;
    int key2 = 0;
    int key3 = 256;
    int val1 = 0;
    int val2 = 0;

    insert_and_test(&map, &key1, &val1, true);
	insert_and_test(&map, &key1, &val2, false);
	insert_and_test(&map, &key2, &val2, false);
	insert_and_test(&map, &key3, &val2, true);

    map_destroy(&map);

    map = map_create(sizeof(char*), sizeof(int), true, false,
                     MAP_MIN_CAP, hash_strings, cmp_strings, free, NULL);

    for (int i = 0; i < N; ++i) {
        char buffer[1024] = { 0 };
        snprintf(buffer, sizeof buffer, "%d", i);
        map_insert(&map, strdup(buffer), i);
    }

    for (int i = 0; i < N; ++i) {
        char buffer[1024] = { 0 };
        snprintf(buffer, sizeof buffer, "%d", i);
        struct map_tuple t = map_find(&map, (char*)buffer);
        TEST_ASSERT(t.key != NULL && t.value != NULL);
        TEST_ASSERT(strcmp(buffer, *(char**)t.key) == 0);
        TEST_ASSERT(*(int*)t.value == i);
        map_replace(&map, (char*)buffer, i + 2025);
    }

    for (int i = 0; i < N; ++i) {
        char buffer[1024] = { 0 };
        snprintf(buffer, sizeof buffer, "%d", i);
        struct map_tuple t = map_find(&map, (char*)buffer);
        TEST_ASSERT(t.key != NULL && t.value != NULL);
        TEST_ASSERT(strcmp(buffer, *(char**)t.key) == 0);
        TEST_ASSERT(*(int*)t.value == i + 2025);
    }
    map_destroy(&map);
    free(val_arr);
    free(key_arr);
}


void test_map_delete(void) {
    int N = 10000;
    struct map *map1 = map_create(sizeof(struct person), sizeof(int), 
                                  false, false, MAP_MIN_CAP, hash_person_id,
                                  cmp_person_id, person_destructor, NULL);

    struct map *map2 = map_create(sizeof(int*), sizeof(int*), true, 
                                  true, MAP_MIN_CAP, hash_point_ints,
                                  cmp_point_ints, free, free);


    int **key_array = malloc(sizeof(int*) * N);
    int **val_array = malloc(sizeof(int*) * N);
    for (int i = 0; i < N; ++i) {
        key_array[i] = create_int(i);
        val_array[i] = create_int(i);
        map_insert(&map2, key_array[i], val_array[i]);
        if (i % (N / 20) == 0)
            remove_and_test(&map2, key_array[i], true);
    }
    remove_and_test(&map2, &(int){N}, false);

    for (int i = 0; i < N; ++i)
        if (i % (N / 20) != 0)
            remove_and_test(&map2, key_array[i], true);

    remove_and_test(&map2, &(int){0}, false);

    for (int i = 0; i < N; ++i)
        remove_and_test(&map2, &(int){i}, false);


    struct person *persons = create_persons(N);
    for (int i = 0; i < N; ++i)
        map_insert(&map1, persons[i], i + N);

    for (int i = 0; i < N; ++i) {
        struct map_tuple t = map_find(&map1, persons[i]);
        TEST_ASSERT(t.key != NULL && t.value != NULL);
        TEST_ASSERT(eq_persons(&persons[i], (struct person*)t.key));
        TEST_ASSERT(*(int*)t.value == i + N);
    }

    for (int i = 0; i < N; ++i) {
        if (i % 5 == 0) 
            map_delete(&map1, persons[i]);
    }

    for (int i = 0; i < N; ++i) {
        struct map_tuple t = map_find(&map1, ((struct person){.id = i}));
        if (i % 5 == 0) {
            TEST_ASSERT(t.key == NULL && t.value == NULL);
        } else {
            TEST_ASSERT(eq_persons(&persons[i], (struct person*)t.key));
            TEST_ASSERT(*(int*)t.value == i + N);
            map_replace(&map1, persons[i], 2025 + i);
        }
    }

    for (int i = 0; i < N; ++i) {
        if (i % 5 == 0)
            continue;
        struct map_tuple t = map_find(&map1, persons[i]);
        TEST_ASSERT(t.key != NULL && t.value != NULL);
        TEST_ASSERT(eq_persons(&persons[i], (struct person*)t.key));
        TEST_ASSERT(*(int*)t.value == 2025 + i);
    }    
    map_destroy(&map1);
    map_destroy(&map2);
    free(key_array);
    free(val_array);
    free(persons);
}




TEST_LIST = {
    "test_map_create", test_map_create,
    "test_map_insert", test_map_insert,
    "test_map_delete", test_map_delete,
    NULL,               NULL 
};