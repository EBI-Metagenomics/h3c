#ifndef ZC_H
#define ZC_H

#include <inttypes.h>
#include <limits.h>
#include <stddef.h>
#include <stdint.h>

#if SHRT_MAX == INT8_MAX
#define ZC_BYTES_PER_SHORT 1
#define ZC_SHORT_WIDTH 8
#elif SHRT_MAX == INT16_MAX
#define ZC_BYTES_PER_SHORT 2
#define ZC_SHORT_WIDTH 16
#elif SHRT_MAX == INT32_MAX
#define ZC_BYTES_PER_SHORT 4
#define ZC_SHORT_WIDTH 32
#elif SHRT_MAX == INT64_MAX
#define ZC_BYTES_PER_SHORT 8
#define ZC_SHORT_WIDTH 64
#else
#error "Cannot determine size of short"
#endif

#if INT_MAX == INT8_MAX
#define ZC_BYTES_PER_INT 1
#define ZC_INT_WIDTH 8
#elif INT_MAX == INT16_MAX
#define ZC_BYTES_PER_INT 2
#define ZC_INT_WIDTH 16
#elif INT_MAX == INT32_MAX
#define ZC_BYTES_PER_INT 4
#define ZC_INT_WIDTH 32
#elif INT_MAX == INT64_MAX
#define ZC_BYTES_PER_INT 8
#define ZC_INT_WIDTH 64
#else
#error "Cannot determine size of int"
#endif

#if LONG_MAX == INT8_MAX
#define ZC_BYTES_PER_LONG 1
#define ZC_LONG_WIDTH 8
#elif LONG_MAX == INT16_MAX
#define ZC_BYTES_PER_LONG 2
#define ZC_LONG_WIDTH 16
#elif LONG_MAX == INT32_MAX
#define ZC_BYTES_PER_LONG 4
#define ZC_LONG_WIDTH 32
#elif LONG_MAX == INT64_MAX
#define ZC_BYTES_PER_LONG 8
#define ZC_LONG_WIDTH 64
#else
#error "Cannot determine size of long"
#endif

enum
{
    ZC_WINDOWS,
    ZC_UNIX,
    ZC_MACOS,
};

#if defined(_WIN16) || defined(_WIN32) || defined(_WIN64) || defined(__WIN32__)
#define ZC_OS ZS_WINDOWS
#elif defined(__unix__) || defined(__unix)
#define ZC_OS ZS_UNIX
#elif defined(__APPLE__)
#define ZC_OS ZC_MACOS
#endif

#ifdef MS_WINDOWS
#define ZC_PATH_SEP '\\'
#endif

#ifndef ZC_PATH_SEP
#define ZC_PATH_SEP '/'
#endif

#define zc_array_size(arr) (sizeof(arr) / sizeof((arr)[0]))

/**
 * zc_container_of - cast a member of a structure out to the containing
 * structure
 * @ptr:	the pointer to the member.
 * @type:	the type of the container struct this is embedded in.
 * @member:	the name of the member within the struct.
 *
 */
#define zc_container_of(ptr, type, member)                                     \
    ({                                                                         \
        char *__mptr = (char *)(ptr);                                          \
        ((type *)(__mptr - offsetof(type, member)));                           \
    })

/**
 * zc_container_of_safe - cast a member of a structure out to the containing
 * structure
 * @ptr:	the pointer to the member.
 * @type:	the type of the container struct this is embedded in.
 * @member:	the name of the member within the struct.
 *
 * Return NULL if ptr is NULL.
 */
#define zc_container_of_safe(ptr, type, member)                                \
    ({                                                                         \
        char *__mptr = (char *)(ptr);                                          \
        __mptr == NULL ? (type *)__mptr                                        \
                       : ((type *)(__mptr - offsetof(type, member)));          \
    })

uint16_t zc_byteswap16(uint16_t x);
uint32_t zc_byteswap32(uint32_t x);
uint64_t zc_byteswap64(uint64_t x);

uint16_t zc_htons(uint16_t);
uint32_t zc_htonl(uint32_t);
uint64_t zc_htonll(uint64_t);

uint16_t zc_ntohs(uint16_t);
uint32_t zc_ntohl(uint32_t);
uint64_t zc_ntohll(uint64_t);

void *zc_reallocf(void *ptr, size_t size);
void zc_bzero(void *dst, size_t dsize);

struct mempool;

struct mempool *zc_mempool_new(unsigned bits, size_t object_size);
void zc_mempool_del(struct mempool *);

void *zc_mempool_new_object(struct mempool *);
void zc_mempool_del_object(struct mempool *, void *object);

char *zc_basename(char const *path);
char *zc_dirname(char const *path);

char *zc_strdup(const char *s);
size_t zc_strlcat(char *dst, char const *src, size_t dsize);
size_t zc_strlcpy(char *dst, char const *src, size_t dsize);

long long zc_strto_llong(const char *restrict, char **restrict, int);
long zc_strto_long(const char *restrict, char **restrict, int);
int zc_strto_int(const char *restrict, char **restrict, int);
short zc_strto_short(const char *restrict, char **restrict, int);

unsigned long long zc_strto_ullong(const char *restrict, char **restrict, int);
unsigned long zc_strto_ulong(const char *restrict, char **restrict, int);
unsigned int zc_strto_uint(const char *restrict, char **restrict, int);
unsigned short zc_strto_ushort(const char *restrict, char **restrict, int);

int64_t zc_strto_int64(const char *restrict, char **restrict, int);
int32_t zc_strto_int32(const char *restrict, char **restrict, int);
int16_t zc_strto_int16(const char *restrict, char **restrict, int);
int8_t zc_strto_int8(const char *restrict, char **restrict, int);

uint64_t zc_strto_uint64(const char *restrict, char **restrict, int);
uint32_t zc_strto_uint32(const char *restrict, char **restrict, int);
uint16_t zc_strto_uint16(const char *restrict, char **restrict, int);
uint8_t zc_strto_uint8(const char *restrict, char **restrict, int);

float zc_strto_float(const char *restrict, char **restrict);
double zc_strto_double(const char *restrict, char **restrict);
long double zc_strto_ldouble(const char *restrict, char **restrict);

#endif
