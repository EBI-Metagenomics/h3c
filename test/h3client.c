#include "h3client/h3client.h"
#include "file_hash.h"

#ifdef _POSIX_C_SOURCE
#if _POSIX_C_SOURCE < 200112L
#undef _POSIX_C_SOURCE
#define _POSIX_C_SOURCE 200112L
#endif
#else
#define _POSIX_C_SOURCE 200112L
#endif

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

static char const *h3master_address(void)
{
    char const *addr = getenv("H3CLIENT_MASTER_ADDRESS");
    return addr ? addr : "127.0.0.1";
}

#define XFAIL(msg, file, line)                                                 \
    do                                                                         \
    {                                                                          \
        exit_status = 1;                                                       \
        printf("%s:%d: ", file, line);                                         \
        printf("failed to ");                                                  \
        printf(msg);                                                           \
        putchar('\n');                                                         \
        goto cleanup;                                                          \
    } while (0);

#define FAIL(msg) XFAIL(msg, __FILE__, __LINE__)

static struct h3c_result *result = 0;
static int exit_status = 0;
static char const cmd[] = "--hmmdb 1 --acc --cut_ga";

static int test_open_close_connection(uint16_t ross_id)
{
    if (h3c_open(h3master_address(), 51370 + ross_id)) FAIL("h3c_open");
    if (h3c_close()) FAIL("h3c_close");

cleanup:
    return exit_status;
}

static int create_result_ross(uint16_t ross_id)
{
    if (h3c_open(h3master_address(), 51370 + ross_id)) FAIL("h3c_open");

    FILE *file = 0;
    if (!(file = fopen(ASSETS "/ross.fasta", "r"))) FAIL("fopen");
    if (!(result = h3c_result_new())) FAIL("h3c_result_new")
    if (h3c_call(cmd, file, result)) FAIL("h3c_call");
    fclose(file);

    if (h3c_close()) FAIL("h3c_close");

cleanup:
    return exit_status;
}

static int check_hash(char const *filepath, int64_t hash, char const *source,
                      int line)
{
    FILE *file = 0;
    int64_t h = 0;

    if (!(file = fopen(filepath, "rb"))) XFAIL("fopen", source, line);
    if (!file_hash(file, &h)) XFAIL("file_hash", source, line);
    if (h != hash) XFAIL("match file hash", source, line);
    fclose(file);

cleanup:
    return exit_status;
}

#define CHECK_HASH(F, H) check_hash(F, H, __FILE__, __LINE__)

static int test_pack_result(uint16_t ross_id)
{
    if (create_result_ross(ross_id)) goto cleanup;

    FILE *file = 0;
    if (!(file = fopen(TMPDIR "/h3result.mp", "wb"))) FAIL("fopen");
    if (h3c_result_pack(result, file)) FAIL("h3c_result_pack");
    fclose(file);

    if (CHECK_HASH(TMPDIR "/h3result.mp", -7580715214164890760LL)) goto cleanup;

cleanup:
    h3c_result_del(result);
    return exit_status;
}

static int test_print_targets(uint16_t ross_id)
{
    if (create_result_ross(ross_id)) goto cleanup;

    FILE *file = 0;
    if (!(file = fopen(TMPDIR "/targets.txt", "wb"))) FAIL("fopen");
    h3c_result_print_targets(result, file);
    fclose(file);

    if (CHECK_HASH(TMPDIR "/targets.txt", 2235430570033520642LL)) goto cleanup;

cleanup:
    h3c_result_del(result);
    return exit_status;
}

static int test_print_domains(uint16_t ross_id)
{
    if (create_result_ross(ross_id)) goto cleanup;

    FILE *file = 0;
    if (!(file = fopen(TMPDIR "/domains.txt", "wb"))) FAIL("fopen");
    h3c_result_print_domains(result, file);
    fclose(file);

    if (CHECK_HASH(TMPDIR "/domains.txt", 450185627565076573LL)) goto cleanup;

cleanup:
    h3c_result_del(result);
    return exit_status;
}

int main(void)
{
    if (test_open_close_connection(5)) goto cleanup;
    if (test_pack_result(5)) goto cleanup;
    if (test_print_targets(5)) goto cleanup;
    if (test_print_domains(5)) goto cleanup;

cleanup:
    return exit_status;
}
