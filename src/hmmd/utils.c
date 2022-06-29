#include "hmmd/utils.h"
#include "c_toolbelt/c_toolbelt.h"
#include "h3client/rc.h"
#include <stddef.h>
#include <string.h>

union num64
{
    int64_t i;
    uint64_t u;
    double f;
};

union num32
{
    int32_t i;
    uint32_t u;
    float f;
};

static union num64 eatnum64(unsigned char const **data)
{
    union num64 ui = {.u = 0};
    memcpy(&ui.u, *data, sizeof(ui.u));
    *data += sizeof(ui.u);
    ui.u = ctb_ntohll(ui.u);
    return ui;
}

static union num32 eatnum32(unsigned char const **data)
{
    union num32 ui = {.u = 0};
    memcpy(&ui.u, *data, sizeof(ui.u));
    *data += sizeof(ui.u);
    ui.u = ctb_ntohl(ui.u);
    return ui;
}

uint64_t eatu64(unsigned char const **data) { return eatnum64(data).u; }

int64_t eati64(unsigned char const **data) { return eatnum64(data).i; }

uint32_t eatu32(unsigned char const **data) { return eatnum32(data).u; }

int32_t eati32(unsigned char const **data) { return eatnum32(data).i; }

uint8_t eatu8(unsigned char const **data)
{
    uint8_t u8 = 0;
    memcpy(&u8, *data, sizeof(u8));
    *data += sizeof(u8);
    return u8;
}

double eatf64(unsigned char const **data) { return eatnum64(data).f; }

float eatf32(unsigned char const **data) { return eatnum32(data).f; }

enum h3c_rc eatstr(char **dst, unsigned char const **data)
{
    size_t size = strlen((char const *)*data) + 1;
    if (!(*dst = ctb_realloc(*dst, size))) return H3C_NOT_ENOUGH_MEMORY;
    memcpy(*dst, *data, size);
    *data += size;
    return H3C_OK;
}

char *strskip(char **str)
{
    char *tmp = *str;
    *str += strlen(*str) + 1;
    return tmp;
}

bool expect_n_strings(size_t size, char const *ptr, unsigned n)
{
    char const *end = ptr + size;
    unsigned i = 0;
    while (i < n && ptr < end)
    {
        while (ptr < end && *ptr)
            ++ptr;

        if (ptr < end) ++i;
    }
    return i == n;
}
