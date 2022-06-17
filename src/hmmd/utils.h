#ifndef HMMD_UTILS_H
#define HMMD_UTILS_H

#include <stdint.h>

uint64_t eat64(unsigned char const **data);
uint32_t eat32(unsigned char const **data);
uint8_t eat8(unsigned char const **data);
enum h3c_rc eatstr(char **dst, unsigned char const **data);

char *strskip(char **str);

static inline void to_double(double *dst, uint64_t h64)
{
    *dst = *((double *)&h64);
}

static inline void to_float(float *dst, uint32_t h32)
{
    *dst = *((float *)&h32);
}

#endif
