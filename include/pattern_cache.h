#ifndef PATTERN_CACHE_H
#define PATTERN_CACHE_H

#include "bricllm.h"
#include <time.h>

#define CACHE_SIZE 50

typedef struct {
    char *query;
    char *role;
    char *language;
    ResponsePattern *pattern;
    time_t timestamp;
    int hit_count;
} CacheEntry;

typedef struct {
    CacheEntry entries[CACHE_SIZE];
    int next_slot;
    int total_hits;
    int total_misses;
} PatternCache;

void init_pattern_cache(void);
ResponsePattern *cache_lookup(const char *query, const char *role, const char *language);
void cache_store(const char *query, const char *role, const char *language, ResponsePattern *pattern);
void cache_stats(void);
void cleanup_cache(void);

#endif // PATTERN_CACHE_H
