#ifndef UTILS_H
#define UTILS_H

#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>

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

struct lip_file;

bool expect_key(struct lip_file *f, char const *key);
bool expect_array_size(struct lip_file *f, unsigned size);
bool expect_map_size(struct lip_file *f, unsigned size);

enum h3c_rc read_string(struct lip_file *f, char **str);

char *strxdup(char *dst, char const *src);

#define MAX(a, b) (((a) > (b)) ? (a) : (b))

static inline unsigned zero_clip(int x) { return x > 0 ? (unsigned)x : 0; }

#define DEL(ptr)                                                               \
    do                                                                         \
    {                                                                          \
        free((ptr));                                                           \
        (ptr) = 0;                                                             \
    } while (0);

#endif
