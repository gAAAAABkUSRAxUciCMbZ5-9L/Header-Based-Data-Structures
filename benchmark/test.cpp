/* test.cpp */
#include <iostream>
#include <cstring>
#include <cstdio>
#include <random>
#include <boost/unordered/unordered_flat_map.hpp>

#include "map.h"


struct times {
    double insert_time;
    double lookup_time;
};


struct djb2_hash {
    std::size_t operator()(const char *str) const {
        std::size_t hash = 5381;
        int c;
        while ((c = *str++))
            hash = ((hash << 5) + hash) + c;
        return hash;
    }
};

struct cstr_equal {
    bool operator()(const char *a, const char *b) const {
        return std::strcmp(a, b) == 0;
    }
};


static inline int cmp_strings(void *a, void *b) {
    return strcmp(*(char**)a, *(char**)b);
}


static uint64_t hash_strings(void *v) {
    uint64_t hash = 5381;
    char *str = *(char**)v;

    int c;
    while ((c = *str++))
        hash = ((hash << 5) + hash) + c;

    return hash;
}


static inline uint64_t hash_ints(void *v) {
    return *(int64_t*)v;
}


static inline int cmp_ints(void *a, void *b) {
    int64_t aa = *(int64_t*)a;
    int64_t bb = *(int64_t*)b;
    return (aa > bb) - (aa < bb);
}


static int64_t *generate_random_keys(size_t n, int seed = 42) {
    std::mt19937 rng(seed);
    std::uniform_int_distribution<int64_t> dist(1, n * 10);
    int64_t *keys = new int64_t[n];
    std::unordered_map<int64_t, int64_t> used;

    for (size_t i = 0; i < n;) {
        int64_t val = dist(rng);
        if (used.find(val) != used.end())
            continue;
        keys[i++] = val;
        used.insert({val, i});
    }
    return keys;
}


static struct times benchmark_int_boost_map(size_t N, int64_t *keys) {
    struct times times = {};
    boost::unordered_flat_map<int64_t, int64_t> map;

    auto start = std::chrono::high_resolution_clock::now();
    for (int64_t i = 0; i < N; ++i)
        map[keys[i]] = i;
    auto end = std::chrono::high_resolution_clock::now();

    times.insert_time = std::chrono::duration<double>(end - start).count();

    start = std::chrono::high_resolution_clock::now();
    for (int64_t i = 0; i < 2 * N; ++i)
        volatile auto it = map.find(i < N ? keys[i] : -i);
    end = std::chrono::high_resolution_clock::now();
 
    times.lookup_time = std::chrono::duration<double>(end - start).count();

    return times;
}


static struct times benchmark_str_boost_map(size_t N, char **keys) {
    struct times times = { };
    boost::unordered_flat_map<const char*, int, djb2_hash, cstr_equal> map;

    auto start = std::chrono::high_resolution_clock::now();
    for (int64_t i = 0; i < N; ++i)
        map[keys[i]] = i;
    auto end = std::chrono::high_resolution_clock::now();

    times.insert_time = std::chrono::duration<double>(end - start).count();

    start = std::chrono::high_resolution_clock::now();
    for (int64_t i = 0; i < 2 * N; ++i)
        volatile auto it = map.find(i < N ? keys[i] : "i");

    end = std::chrono::high_resolution_clock::now();

    times.lookup_time = std::chrono::duration<double>(end - start).count();

    return times;
}


static struct times benchmark_str_impl_map(size_t N, char **keys) {
    struct times times = { };
    struct map *map = map_create(sizeof(char*), sizeof(int64_t), true, false,
                                 MAP_MIN_CAP, hash_strings, cmp_strings, NULL, NULL);

    auto start = std::chrono::high_resolution_clock::now();
    for (int64_t i = 0; i < N; ++i)
        map_insert(&map, keys[i], i);
    auto end = std::chrono::high_resolution_clock::now();

    times.insert_time = std::chrono::duration<double>(end - start).count();

    start = std::chrono::high_resolution_clock::now();
    for (int64_t i = 0; i < 2 * N; ++i) {
        volatile struct map_tuple t = map_find(&map, i< N ? keys[i] : "i");
    }
    end = std::chrono::high_resolution_clock::now();

    times.lookup_time = std::chrono::duration<double>(end - start).count();

    map_destroy(&map);
    return times;
}


static struct times benchmark_int_impl_map(size_t N, int64_t *keys) {
    struct times times = { };
    struct map *map = map_create(sizeof(int64_t), sizeof(int64_t), false, false,
                                 MAP_MIN_CAP, hash_ints, cmp_ints, NULL, NULL);
    auto start = std::chrono::high_resolution_clock::now();
    for (int64_t i = 0; i < N; ++i)
        map_insert(&map, keys[i], i);
    auto end = std::chrono::high_resolution_clock::now();

    times.insert_time = std::chrono::duration<double>(end - start).count();


    start = std::chrono::high_resolution_clock::now();
    for (int64_t i = 0; i < 2 * N; ++i) {
        volatile struct map_tuple t = map_find(&map, i < N ? keys[i] : -i);
    }
    end = std::chrono::high_resolution_clock::now();
    map_destroy(&map);

    times.lookup_time = std::chrono::duration<double>(end - start).count();

    return times;
}


void test_strings(uint64_t N) {
    int64_t *keys = generate_random_keys(N);
    char **str_keys = (char**)malloc(N * sizeof(char*));
    for (uint64_t i = 0UL; i < N; ++i) {
        char buffer[1024] = {0};
        snprintf(buffer, sizeof buffer, "af%ludl", keys[i]);
        str_keys[i] = strdup(buffer);
    }

    struct times t1 = benchmark_str_impl_map(N, str_keys);
    struct times t2 = benchmark_str_boost_map(N, str_keys); 
    printf("Benchmark run with %lu C strings elements as keys\n", N);
    printf("Custom made hash based map:\n"
           "  1) Insertion Time(s): %.6f\n"
           "  2) Lookup Time(s)   : %.6f\n\n",
           t1.insert_time, t1.lookup_time);
           
    printf("Boost unordered_flat_map:\n"
           "  1) Insertion Time(s): %.6f\n"
           "  2) Lookup Time(s)   : %.6f\n\n",
           t2.insert_time, t2.lookup_time); 


    for (uint64_t i = 0UL; i < N; ++i)
        free(str_keys[i]);
    free(str_keys);
    free(keys);
}


void test_ints(uint64_t N) {
    int64_t *keys = generate_random_keys(N);
    struct times t1 = benchmark_int_impl_map(N, keys);
    struct times t2 = benchmark_int_boost_map(N, keys); 
    printf("Benchmark run with %lu integer elements as keys\n", N);
    printf("Custom made hash based map:\n"
           "  1) Insertion Time(s): %.6f\n"
           "  2) Lookup Time(s)   : %.6f\n\n",
           t1.insert_time, t1.lookup_time);
           
    printf("Boost unordered_flat_map:\n"
           "  1) Insertion Time(s): %.6f\n"
           "  2) Lookup Time(s)   : %.6f\n\n",
           t2.insert_time, t2.lookup_time); 

    free(keys);
}


int main(void) {
    uint64_t N = 1e8;
    test_strings(N);
    test_ints(N);
    return 0;
}
