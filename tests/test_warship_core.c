#include "warship_core.h"

#include <stdio.h>
#include <string.h>

static int failures = 0;
static int checks = 0;

#define CHECK(expr) do { \
    ++checks; \
    if (!(expr)) { \
        fprintf(stderr, "FAIL line %d: %s\n", __LINE__, #expr); \
        ++failures; \
    } \
} while (0)

int main(void)
{
    char exact[WARSHIP_CORE_BANNER_CAPACITY];
    char oversized[WARSHIP_CORE_BANNER_CAPACITY + 2u];
    char undersized[WARSHIP_CORE_BANNER_CAPACITY - 1u];
    char before[sizeof undersized];

    memset(exact, 0x5a, sizeof exact);
    CHECK(warship_core_banner(exact, sizeof exact) == WARSHIP_CORE_OK);
    CHECK(strcmp(exact, "CYD WARSHIP T01") == 0);
    CHECK(exact[sizeof exact - 1u] == '\0');

    memset(oversized, 0x5a, sizeof oversized);
    CHECK(warship_core_banner(oversized, sizeof oversized) == WARSHIP_CORE_OK);
    CHECK(strcmp(oversized, "CYD WARSHIP T01") == 0);
    CHECK(oversized[WARSHIP_CORE_BANNER_CAPACITY] == 0x5a);
    CHECK(oversized[WARSHIP_CORE_BANNER_CAPACITY + 1u] == 0x5a);

    memset(undersized, 0x5a, sizeof undersized);
    memcpy(before, undersized, sizeof before);
    CHECK(warship_core_banner(undersized, sizeof undersized) == WARSHIP_CORE_INVALID_ARGUMENT);
    CHECK(memcmp(undersized, before, sizeof before) == 0);
    CHECK(warship_core_banner(undersized, 0u) == WARSHIP_CORE_INVALID_ARGUMENT);
    CHECK(memcmp(undersized, before, sizeof before) == 0);
    CHECK(warship_core_banner(NULL, WARSHIP_CORE_BANNER_CAPACITY) == WARSHIP_CORE_INVALID_ARGUMENT);
    CHECK(warship_core_banner(NULL, 0u) == WARSHIP_CORE_INVALID_ARGUMENT);

    printf("T01 core C99: %d checks, %d failures\n", checks, failures);
    return failures == 0 ? 0 : 1;
}
