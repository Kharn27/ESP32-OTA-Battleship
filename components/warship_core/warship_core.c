#include "warship_core.h"

#include <string.h>

static const char banner[] = "CYD WARSHIP T01";

typedef char banner_capacity_check[
    sizeof banner == WARSHIP_CORE_BANNER_CAPACITY ? 1 : -1];

int warship_core_banner(char *out, size_t capacity)
{
    if (out == NULL || capacity < sizeof banner) {
        return WARSHIP_CORE_INVALID_ARGUMENT;
    }

    memcpy(out, banner, sizeof banner);
    return WARSHIP_CORE_OK;
}
