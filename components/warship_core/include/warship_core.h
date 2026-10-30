#ifndef WARSHIP_CORE_H
#define WARSHIP_CORE_H

#include <stddef.h>

/* Includes the trailing NUL. No ESP-IDF/FreeRTOS dependency. */
#define WARSHIP_CORE_BANNER_CAPACITY 16u
#define WARSHIP_CORE_OK 0
#define WARSHIP_CORE_INVALID_ARGUMENT (-1)

/*
 * Write "CYD WARSHIP T01" and the trailing NUL to out.
 * Returns OK when out != NULL and capacity >= WARSHIP_CORE_BANNER_CAPACITY.
 * On error, performs no writes. Requires no allocation or global state.
 */
int warship_core_banner(char *out, size_t capacity);

#endif
