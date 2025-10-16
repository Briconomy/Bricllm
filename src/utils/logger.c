#include "../../include/bricllm.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <time.h>
#include <string.h>

char *get_timestamp(void) {
    static char timestamp[20];
    time_t now = time(NULL);
    struct tm *tm_info = localtime(&now);

    strftime(timestamp, sizeof(timestamp), "%Y-%m-%d %H:%M:%S", tm_info);
    return timestamp;
}

void log_message(const char *level, const char *format, ...) {
    if (!level || !format) return;

    char *timestamp = get_timestamp();

    va_list args;
    va_start(args, format);

    printf("[%s] [%s] ", timestamp, level);
    vprintf(format, args);
    printf("\n");

    va_end(args);
}

char *generate_uuid(void) {
    char *uuid = malloc(37);
    if (!uuid) return NULL;

    snprintf(uuid, 37, "%04x%04x-%04x-%04x-%04x-%04x%04x%04x",
             rand() % 0xFFFF, rand() % 0xFFFF,
             rand() % 0xFFFF,
             (rand() % 0x0FFF) | 0x4000,
             (rand() % 0x3FFF) | 0x8000,
             rand() % 0xFFFF, rand() % 0xFFFF, rand() % 0xFFFF);

    return uuid;
}