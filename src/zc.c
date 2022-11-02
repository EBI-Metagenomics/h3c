#include "zc.h"
#include <assert.h>
#include <errno.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

uint16_t zc_byteswap16(uint16_t x) { return (uint16_t)(x << 8 | x >> 8); }

uint32_t zc_byteswap32(uint32_t x)
{
    return (uint32_t)(x >> 24 | (x >> 8 & 0xff00) | (x << 8 & 0xff0000) |
                      x << 24);
}

uint64_t zc_byteswap64(uint64_t x)
{
    return (uint64_t)((zc_byteswap32(x) + 0ULL) << 32 | zc_byteswap32(x >> 32));
}

enum
{
    ZC_ENDIAN_LITTLE = 1234,
    ZC_ENDIAN_BIG = 4321,
};

#ifdef __STDC_ENDIAN_NATIVE__
#if __STDC_ENDIAN_NATIVE__ == __STDC_ENDIAN_LITTLE__
#define ZC_ENDIAN_NATIVE ZC_ENDIAN_LITTLE
#elif __STDC_ENDIAN_NATIVE__ == __STDC_ENDIAN_BIG__
#define ZC_ENDIAN_NATIVE ZC_ENDIAN_BIG
#else
#error "Unknown native byte order"
#endif
#endif

#ifdef ZC_ENDIAN_NATIVE

#if ZC_ENDIAN_NATIVE == ZC_ENDIAN_LITTLE

uint16_t zc_htons(uint16_t x) { return zc_byteswap16(x); }
uint32_t zc_htonl(uint32_t x) { return zc_byteswap32(x); }
uint64_t zc_htonll(uint64_t x) { return zc_byteswap64(x); }

uint16_t zc_ntohs(uint16_t x) { return zc_htons(x); }
uint32_t zc_ntohl(uint32_t x) { return zc_htonl(x); }
uint64_t zc_ntohll(uint64_t x) { return zc_htonll(x); }

#else
uint16_t zc_htons(uint16_t x) { return x; }
uint32_t zc_htonl(uint32_t x) { return x; }
uint64_t zc_htonll(uint64_t x) { return x; }

uint16_t zc_ntohs(uint16_t x) { return x; }
uint32_t zc_ntohl(uint32_t x) { return x; }
uint64_t zc_ntohll(uint64_t x) { return x; }
#endif

#else

static int zc_endianness(void)
{
    union
    {
        uint32_t value;
        uint8_t data[sizeof(uint32_t)];
    } number;

    number.data[0] = 0x00;
    number.data[1] = 0x01;
    number.data[2] = 0x02;
    number.data[3] = 0x03;

    switch (number.value)
    {
    case (uint32_t)(0x00010203):
        return ZC_ENDIAN_BIG;
    case (uint32_t)(0x03020100):
        return ZC_ENDIAN_LITTLE;
    default:
        exit(1);
        return -1;
    }
}

uint16_t zc_htons(uint16_t x)
{
    if (zc_endianness() == ZC_ENDIAN_BIG) return x;
    return zc_byteswap16(x);
}

uint32_t zc_htonl(uint32_t x)
{
    if (zc_endianness() == ZC_ENDIAN_BIG) return x;
    return zc_byteswap32(x);
}

uint64_t zc_htonll(uint64_t x)
{
    if (zc_endianness() == ZC_ENDIAN_BIG) return x;
    return zc_byteswap64(x);
}

uint16_t zc_ntohs(uint16_t x) { return zc_htons(x); }
uint32_t zc_ntohl(uint32_t x) { return zc_htonl(x); }
uint64_t zc_ntohll(uint64_t x) { return zc_htonll(x); }

#endif

void *zc_reallocf(void *ptr, size_t size)
{
    assert(size > 0);
    void *new_ptr = realloc(ptr, size);

    if (!new_ptr) free(ptr);
    return new_ptr;
}

void zc_bzero(void *dst, size_t dsize) { memset(dst, 0, dsize); }

struct proxy
{
    struct proxy *next_free;
};

struct block
{
    struct block *next_block;
};

struct mempool
{
    size_t object_size;
    size_t block_size;
    struct proxy *free_list;
    size_t allocated_count;
    struct block *blocks;
};

/* chunk is the memory allocated to fit both proxy and its object */
static inline size_t object_chunk_size(struct mempool const *mp)
{
    return (sizeof(struct proxy) + (mp)->object_size);
}

static inline struct proxy *get_proxy_from_object(void *object)
{
    return (struct proxy *)(object)-1;
}

static inline void *get_object_from_proxy(struct proxy *proxy)
{
    return ((struct proxy *)(proxy)) + 1;
}

struct mempool *zc_mempool_new(unsigned bits, size_t object_size)
{
    struct mempool *mp = malloc(sizeof(struct mempool));
    if (!mp) return 0;
    mp->object_size = object_size;
    mp->block_size = 1 << (bits);
    mp->free_list = 0;
    mp->allocated_count = 0;
    mp->blocks = 0;
    assert(object_chunk_size(mp) <= mp->block_size);
    return mp;
}

void zc_mempool_del(struct mempool *mp)
{
    assert(mp->allocated_count == 0);

    struct block *curr = mp->blocks;
    while (curr)
    {
        struct block *next = curr->next_block;
        free(curr);
        curr = next;
    }

    free(mp);
}

static void divide_block_into_chunks(struct mempool *mp, struct block *block)
{
    void *offset = block;
    size_t pos = sizeof(struct block);
    while (pos + object_chunk_size(mp) <= mp->block_size)
    {
        struct proxy *proxy = offset + pos;
        proxy->next_free = mp->free_list;
        mp->free_list = proxy;
        pos += object_chunk_size(mp);
    }
}

static bool new_block(struct mempool *mp)
{
    struct block *block = malloc(mp->block_size);
    if (!block) return false;
    block->next_block = mp->blocks;
    mp->blocks = block;

    divide_block_into_chunks(mp, block);
    return true;
}

void *zc_mempool_new_object(struct mempool *mp)
{
    if (!mp->free_list && !new_block(mp)) return 0;

    struct proxy *obj = mp->free_list;
    mp->free_list = obj ? obj->next_free : 0;
    mp->allocated_count++;
    return get_object_from_proxy(obj);
}

void zc_mempool_del_object(struct mempool *mp, void *object)
{
    struct proxy *proxy = get_proxy_from_object(object);
    proxy->next_free = mp->free_list;
    mp->free_list = proxy;
    mp->allocated_count--;
}

// Acknowledgment: gblic
char *zc_basename(char const *path)
{
    char *p = strrchr(path, ZC_PATH_SEP);
    return p ? p + 1 : (char *)path;
}

char *zc_dirname(char const *path)
{
    char *p = strrchr(path, ZC_PATH_SEP);
    while (p > path)
    {
        *p-- = 0;
    }
    return (char *)path;
}

char *zc_strdup(char const *str)
{
    size_t len = strlen(str) + 1;
    void *new = malloc(len);

    if (new == NULL) return NULL;

    return (char *)memcpy(new, str, len);
}

/*
 * Appends src to string dst of size dsize (unlike strncat, dsize is the
 * full size of dst, not space left).  At most dsize-1 characters
 * will be copied.  Always NUL terminates (unless dsize <= strlen(dst)).
 * Returns strlen(src) + MIN(dsize, strlen(initial dst)).
 * If retval >= dsize, truncation occurred.
 *
 * Copyright (c) 1998, 2015 Todd C. Miller <Todd.Miller@courtesan.com>
 */
size_t zc_strlcat(char *dst, char const *src, size_t dsize)
{
    const char *odst = dst;
    const char *osrc = src;
    size_t n = dsize;
    size_t dlen;

    while (n-- != 0 && *dst != '\0')
        dst++;
    dlen = dst - odst;
    n = dsize - dlen;

    if (n-- == 0) return (dlen + strlen(src));
    while (*src != '\0')
    {
        if (n != 0)
        {
            *dst++ = *src;
            n--;
        }
        src++;
    }
    *dst = '\0';

    return (dlen + (src - osrc));
}

/*
 * Copyright (c) 1998, 2015 Todd C. Miller <Todd.Miller@courtesan.com>
 *
 * Permission to use, copy, modify, and distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */
size_t zc_strlcpy(char *dst, char const *src, size_t dsize)
{
    char const *osrc = src;
    size_t nleft = dsize;

    if (nleft != 0)
    {
        while (--nleft != 0)
        {
            if ((*dst++ = *src++) == '\0') break;
        }
    }

    if (nleft == 0)
    {
        if (dsize != 0) *dst = '\0';
        while (*src++)
            ;
    }

    return (size_t)(src - osrc - 1);
}

long long zc_strto_llong(const char *restrict nptr, char **restrict endptr,
                         int base)
{
    errno = 0;
    intmax_t v = strtoimax(nptr, endptr, base);
    if (errno)
    {
        if (v == INTMAX_MAX) return LLONG_MAX;
        if (v == INTMAX_MIN) return LLONG_MIN;
        assert(v == 0);
        return 0;
    }
    if (v > LLONG_MAX)
    {
        errno = ERANGE;
        return LLONG_MAX;
    }
    if (v < LLONG_MIN)
    {
        errno = ERANGE;
        return LLONG_MIN;
    }
    return (long long)v;
}

long zc_strto_long(const char *restrict nptr, char **restrict endptr, int base)
{
    errno = 0;
    intmax_t v = strtoimax(nptr, endptr, base);
    if (errno)
    {
        if (v == INTMAX_MAX) return LONG_MAX;
        if (v == INTMAX_MIN) return LONG_MIN;
        assert(v == 0);
        return 0;
    }
    if (v > LONG_MAX)
    {
        errno = ERANGE;
        return LONG_MAX;
    }
    if (v < LONG_MIN)
    {
        errno = ERANGE;
        return LONG_MIN;
    }
    return (long)v;
}

int zc_strto_int(const char *restrict nptr, char **restrict endptr, int base)
{
    errno = 0;
    intmax_t v = strtoimax(nptr, endptr, base);
    if (errno)
    {
        if (v == INTMAX_MAX) return INT_MAX;
        if (v == INTMAX_MIN) return INT_MIN;
        assert(v == 0);
        return 0;
    }
    if (v > INT_MAX)
    {
        errno = ERANGE;
        return INT_MAX;
    }
    if (v < INT_MIN)
    {
        errno = ERANGE;
        return INT_MIN;
    }
    return (int)v;
}

short zc_strto_short(const char *restrict nptr, char **restrict endptr,
                     int base)
{
    errno = 0;
    intmax_t v = strtoimax(nptr, endptr, base);
    if (errno)
    {
        if (v == INTMAX_MAX) return SHRT_MAX;
        if (v == INTMAX_MIN) return SHRT_MIN;
        assert(v == 0);
        return 0;
    }
    if (v > SHRT_MAX)
    {
        errno = ERANGE;
        return SHRT_MAX;
    }
    if (v < SHRT_MIN)
    {
        errno = ERANGE;
        return SHRT_MIN;
    }
    return (short)v;
}

unsigned long long zc_strto_ullong(const char *restrict nptr,
                                   char **restrict endptr, int base)
{
    errno = 0;
    uintmax_t v = strtoumax(nptr, endptr, base);
    if (errno)
    {
        if (v == UINTMAX_MAX) return ULLONG_MAX;
        assert(v == 0);
        return 0;
    }
    if (v > ULLONG_MAX)
    {
        errno = ERANGE;
        return ULLONG_MAX;
    }
    return (unsigned long long)v;
}

unsigned long zc_strto_ulong(const char *restrict nptr, char **restrict endptr,
                             int base)
{
    errno = 0;
    uintmax_t v = strtoumax(nptr, endptr, base);
    if (errno)
    {
        if (v == UINTMAX_MAX) return ULONG_MAX;
        assert(v == 0);
        return 0;
    }
    if (v > ULONG_MAX)
    {
        errno = ERANGE;
        return ULONG_MAX;
    }
    return (unsigned long)v;
}

unsigned int zc_strto_uint(const char *restrict nptr, char **restrict endptr,
                           int base)
{
    errno = 0;
    uintmax_t v = strtoumax(nptr, endptr, base);
    if (errno)
    {
        if (v == UINTMAX_MAX) return UINT_MAX;
        assert(v == 0);
        return 0;
    }
    if (v > UINT_MAX)
    {
        errno = ERANGE;
        return UINT_MAX;
    }
    return (unsigned int)v;
}

unsigned short zc_strto_ushort(const char *restrict nptr,
                               char **restrict endptr, int base)
{
    errno = 0;
    uintmax_t v = strtoumax(nptr, endptr, base);
    if (errno)
    {
        if (v == UINTMAX_MAX) return USHRT_MAX;
        assert(v == 0);
        return 0;
    }
    if (v > USHRT_MAX)
    {
        errno = ERANGE;
        return USHRT_MAX;
    }
    return (unsigned short)v;
}

int64_t zc_strto_int64(const char *restrict nptr, char **restrict endptr,
                       int base)
{
    errno = 0;
    intmax_t v = strtoimax(nptr, endptr, base);
    if (errno)
    {
        if (v == INTMAX_MAX) return INT64_MAX;
        if (v == INTMAX_MIN) return INT64_MIN;
        assert(v == 0);
        return 0;
    }
    if (v > INT64_MAX)
    {
        errno = ERANGE;
        return INT64_MAX;
    }
    if (v < INT64_MIN)
    {
        errno = ERANGE;
        return INT64_MIN;
    }
    return (int64_t)v;
}

int32_t zc_strto_int32(const char *restrict nptr, char **restrict endptr,
                       int base)
{
    errno = 0;
    intmax_t v = strtoimax(nptr, endptr, base);
    if (errno)
    {
        if (v == INTMAX_MAX) return INT32_MAX;
        if (v == INTMAX_MIN) return INT32_MIN;
        assert(v == 0);
        return 0;
    }
    if (v > INT32_MAX)
    {
        errno = ERANGE;
        return INT32_MAX;
    }
    if (v < INT32_MIN)
    {
        errno = ERANGE;
        return INT32_MIN;
    }
    return (int32_t)v;
}

int16_t zc_strto_int16(const char *restrict nptr, char **restrict endptr,
                       int base)
{
    errno = 0;
    intmax_t v = strtoimax(nptr, endptr, base);
    if (errno)
    {
        if (v == INTMAX_MAX) return INT16_MAX;
        if (v == INTMAX_MIN) return INT16_MIN;
        assert(v == 0);
        return 0;
    }
    if (v > INT16_MAX)
    {
        errno = ERANGE;
        return INT16_MAX;
    }
    if (v < INT16_MIN)
    {
        errno = ERANGE;
        return INT16_MIN;
    }
    return (int16_t)v;
}

int8_t zc_strto_int8(const char *restrict nptr, char **restrict endptr,
                     int base)
{
    errno = 0;
    intmax_t v = strtoimax(nptr, endptr, base);
    if (errno)
    {
        if (v == INTMAX_MAX) return INT8_MAX;
        if (v == INTMAX_MIN) return INT8_MIN;
        assert(v == 0);
        return 0;
    }
    if (v > INT8_MAX)
    {
        errno = ERANGE;
        return INT8_MAX;
    }
    if (v < INT8_MIN)
    {
        errno = ERANGE;
        return INT8_MIN;
    }
    return (int8_t)v;
}

uint64_t zc_strto_uint64(const char *restrict nptr, char **restrict endptr,
                         int base)
{
    errno = 0;
    uintmax_t v = strtoumax(nptr, endptr, base);
    if (errno)
    {
        if (v == UINTMAX_MAX) return UINT64_MAX;
        assert(v == 0);
        return 0;
    }
    if (v > UINT64_MAX)
    {
        errno = ERANGE;
        return UINT64_MAX;
    }
    return (uint64_t)v;
}

uint32_t zc_strto_uint32(const char *restrict nptr, char **restrict endptr,
                         int base)
{
    errno = 0;
    uintmax_t v = strtoumax(nptr, endptr, base);
    if (errno)
    {
        if (v == UINTMAX_MAX) return UINT32_MAX;
        assert(v == 0);
        return 0;
    }
    if (v > UINT32_MAX)
    {
        errno = ERANGE;
        return UINT32_MAX;
    }
    return (uint32_t)v;
}

uint16_t zc_strto_uint16(const char *restrict nptr, char **restrict endptr,
                         int base)
{
    errno = 0;
    uintmax_t v = strtoumax(nptr, endptr, base);
    if (errno)
    {
        if (v == UINTMAX_MAX) return UINT16_MAX;
        assert(v == 0);
        return 0;
    }
    if (v > UINT16_MAX)
    {
        errno = ERANGE;
        return UINT16_MAX;
    }
    return (uint16_t)v;
}

uint8_t zc_strto_uint8(const char *restrict nptr, char **restrict endptr,
                       int base)
{
    errno = 0;
    uintmax_t v = strtoumax(nptr, endptr, base);
    if (errno)
    {
        if (v == UINTMAX_MAX) return UINT8_MAX;
        assert(v == 0);
        return 0;
    }
    if (v > UINT8_MAX)
    {
        errno = ERANGE;
        return UINT8_MAX;
    }
    return (uint8_t)v;
}

float zc_strto_float(const char *restrict nptr, char **restrict endptr)
{
    errno = 0;
    return strtof(nptr, endptr);
}

double zc_strto_double(const char *restrict nptr, char **restrict endptr)
{
    errno = 0;
    return strtod(nptr, endptr);
}

long double zc_strto_ldouble(const char *restrict nptr, char **restrict endptr)
{
    errno = 0;
    return strtold(nptr, endptr);
}
