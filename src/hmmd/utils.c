#include "hmmd/utils.h"
#include "c_toolbelt/c_toolbelt.h"
#include "h3client/rc.h"
#include <stddef.h>
#include <string.h>

uint64_t eatu64(unsigned char const **data)
{
    uint64_t n64 = 0;
    memcpy(&n64, *data, sizeof(uint64_t));
    *data += sizeof(uint64_t);
    return ctb_ntohll(n64);
}

uint32_t eatu32(unsigned char const **data)
{
    uint32_t n32 = 0;
    memcpy(&n32, *data, sizeof(uint32_t));
    *data += sizeof(uint32_t);
    return ctb_ntohl(n32);
}

uint8_t eatu8(unsigned char const **data)
{
    uint8_t n8 = 0;
    memcpy(&n8, *data, sizeof(uint8_t));
    *data += sizeof(uint8_t);
    return n8;
}

double eatf64(unsigned char const **data)
{
    uint64_t u64 = eatu64(data);
    return *((double *)&u64);
}

float eatf32(unsigned char const **data)
{
    uint32_t u32 = eatu32(data);
    return *((float *)&u32);
}

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
