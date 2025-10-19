#include "../include/pattern_cache.h"
#include "../include/bricllm.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

static PatternCache cache;

void init_pattern_cache(void) {
    memset(&cache, 0, sizeof(PatternCache));
    cache.next_slot = 0;
    cache.total_hits = 0;
    cache.total_misses = 0;
    
    log_message("INFO", "Pattern cache initialized (size: %d entries)", CACHE_SIZE);
}

ResponsePattern *cache_lookup(const char *query, const char *role, const char *language) {
    if (!query || !role || !language) return NULL;
    
    char normalized_query[256];
    strncpy(normalized_query, query, sizeof(normalized_query) - 1);
    normalized_query[sizeof(normalized_query) - 1] = '\0';
    
    for (int i = 0; normalized_query[i]; i++) {
        if (normalized_query[i] >= 'A' && normalized_query[i] <= 'Z') {
            normalized_query[i] = normalized_query[i] + 32;
        }
    }
    
    for (int i = 0; i < CACHE_SIZE; i++) {
        CacheEntry *entry = &cache.entries[i];
        
        if (entry->query && 
            strcmp(entry->query, normalized_query) == 0 &&
            strcmp(entry->role, role) == 0 &&
            strcmp(entry->language, language) == 0) {
            
            entry->hit_count++;
            entry->timestamp = time(NULL);
            cache.total_hits++;
            
            log_message("INFO", "Cache HIT: '%s' (hits: %d, cache rate: %.1f%%)", 
                       query, entry->hit_count,
                       (cache.total_hits * 100.0f) / (cache.total_hits + cache.total_misses));
            
            return entry->pattern;
        }
    }
    
    cache.total_misses++;
    log_message("INFO", "Cache MISS: '%s' (cache rate: %.1f%%)", 
               query,
               (cache.total_hits * 100.0f) / (cache.total_hits + cache.total_misses));
    
    return NULL;
}

void cache_store(const char *query, const char *role, const char *language, ResponsePattern *pattern) {
    if (!query || !role || !language || !pattern) return;
    
    char normalized_query[256];
    strncpy(normalized_query, query, sizeof(normalized_query) - 1);
    normalized_query[sizeof(normalized_query) - 1] = '\0';
    
    for (int i = 0; normalized_query[i]; i++) {
        if (normalized_query[i] >= 'A' && normalized_query[i] <= 'Z') {
            normalized_query[i] = normalized_query[i] + 32;
        }
    }
    
    for (int i = 0; i < CACHE_SIZE; i++) {
        if (cache.entries[i].query && 
            strcmp(cache.entries[i].query, normalized_query) == 0) {
            return;
        }
    }
    
    int slot = cache.next_slot;
    time_t oldest_time = time(NULL);
    int oldest_slot = cache.next_slot;
    
    for (int i = 0; i < CACHE_SIZE; i++) {
        if (!cache.entries[i].query) {
            slot = i;
            break;
        }
        if (cache.entries[i].timestamp < oldest_time) {
            oldest_time = cache.entries[i].timestamp;
            oldest_slot = i;
        }
    }
    
    if (cache.entries[slot].query) {
        slot = oldest_slot;
    }
    
    CacheEntry *entry = &cache.entries[slot];
    if (entry->query) {
        free(entry->query);
        free(entry->role);
        free(entry->language);
        if (entry->pattern) {
            free(entry->pattern->response);
            free(entry->pattern->category);
            free(entry->pattern);
        }
    }
    
    entry->query = strdup(normalized_query);
    entry->role = strdup(role);
    entry->language = strdup(language);
    
    entry->pattern = malloc(sizeof(ResponsePattern));
    if (entry->pattern) {
        entry->pattern->response = strdup(pattern->response);
        entry->pattern->category = strdup(pattern->category);
        entry->pattern->keywords = pattern->keywords;
        entry->pattern->keyword_count = pattern->keyword_count;
        entry->pattern->user_role = pattern->user_role;
        entry->pattern->language = pattern->language;
        entry->pattern->confidence_threshold = pattern->confidence_threshold;
    }
    
    entry->timestamp = time(NULL);
    entry->hit_count = 0;
    
    log_message("INFO", "Cache STORE: '%s' in slot %d", query, slot);
    
    cache.next_slot = (cache.next_slot + 1) % CACHE_SIZE;
}

void cache_stats(void) {
    int occupied = 0;
    int total_hits_sum = 0;
    
    for (int i = 0; i < CACHE_SIZE; i++) {
        if (cache.entries[i].query) {
            occupied++;
            total_hits_sum += cache.entries[i].hit_count;
        }
    }
    
    float hit_rate = (cache.total_hits + cache.total_misses > 0) 
        ? (cache.total_hits * 100.0f) / (cache.total_hits + cache.total_misses)
        : 0.0f;
    
    printf("\n=== Pattern Cache Statistics ===\n");
    printf("Cache size: %d entries\n", CACHE_SIZE);
    printf("Occupied slots: %d\n", occupied);
    printf("Total lookups: %d\n", cache.total_hits + cache.total_misses);
    printf("Cache hits: %d\n", cache.total_hits);
    printf("Cache misses: %d\n", cache.total_misses);
    printf("Hit rate: %.1f%%\n", hit_rate);
    printf("Average hits per entry: %.1f\n", 
           occupied > 0 ? (float)total_hits_sum / occupied : 0.0f);
    printf("\n");
}

void cleanup_cache(void) {
    for (int i = 0; i < CACHE_SIZE; i++) {
        CacheEntry *entry = &cache.entries[i];
        if (entry->query) {
            free(entry->query);
            free(entry->role);
            free(entry->language);
            if (entry->pattern) {
                free(entry->pattern->response);
                free(entry->pattern->category);
                free(entry->pattern);
            }
        }
    }
    
    log_message("INFO", "Cache cleanup complete. Final hit rate: %.1f%%",
               (cache.total_hits * 100.0f) / (cache.total_hits + cache.total_misses));
}
