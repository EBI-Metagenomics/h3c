#include "utils.h"
#include "c_toolbelt/c_toolbelt.h"
#include "h3client/rc.h"
#include "lite_pack/lite_pack.h"
#include <assert.h>
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

bool expect_key(struct lip_file *f, char const *key)
{
    char str[16] = {0};
    unsigned size = (unsigned)strlen(key) + 1;
    lip_read_cstr(f, size, str);

    return !lip_file_error(f) && strncmp(str, key, sizeof(str)) == 0;
}

bool expect_array_size(struct lip_file *f, unsigned size)
{
    unsigned sz = 0;
    lip_read_array_size(f, &sz);
    return !lip_file_error(f) && size == sz;
}

bool expect_map_size(struct lip_file *f, unsigned size)
{
    unsigned sz = 0;
    lip_read_map_size(f, &sz);
    return !lip_file_error(f) && size == sz;
}

enum h3c_rc read_string(struct lip_file *f, char **str)
{
    unsigned size = 0;
    if (!lip_read_str_size(f, &size)) return H3C_FAILED_UNPACK;
    if (!(*str = ctb_realloc(*str, size + 1))) return H3C_NOT_ENOUGH_MEMORY;
    if (!lip_read_str_data(f, size, *str)) return H3C_FAILED_UNPACK;
    (*str)[size] = 0;
    return H3C_OK;
}

char *strxdup(char *dst, char const *src)
{
    assert(dst != 0);
    if (!src)
    {
        dst[0] = 0;
        return dst;
    }

    size_t sz = strlen(src);
    void *t = ctb_realloc(dst, sz + 1);
    return t ? memcpy(t, src, sz + 1) : 0;
}
