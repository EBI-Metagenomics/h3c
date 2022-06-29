#ifndef HMMD_UTILS_H
#define HMMD_UTILS_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

uint64_t eatu64(unsigned char const **data);
int64_t eati64(unsigned char const **data);

uint32_t eatu32(unsigned char const **data);
int32_t eati32(unsigned char const **data);

uint8_t eatu8(unsigned char const **data);

double eatf64(unsigned char const **data);
float eatf32(unsigned char const **data);

enum h3c_rc eatstr(char **dst, unsigned char const **data);
char *strskip(char **str);

#define ESCAPE_OVERRUN(rc, cur, end, sz)                                       \
    do                                                                         \
    {                                                                          \
        if ((end) < (cur) + (sz))                                              \
        {                                                                      \
            rc = H3C_FAILED_PARSE;                                             \
            goto cleanup;                                                      \
        }                                                                      \
    } while (0);

bool expect_n_strings(size_t size, char const *ptr, unsigned n);

#endif
