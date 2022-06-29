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

#define ARRAY_SIZE(arr) (sizeof(arr) / sizeof((arr)[0]))

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

static int exit_status = 0;
static char const cmd[] = "--hmmdb 1 --acc --cut_ga";

static int test_open_close_connection(uint16_t ross_id)
{
    if (h3c_open(h3master_address(), 51370 + ross_id + 1)) FAIL("h3c_open");
    if (h3c_close()) FAIL("h3c_close");

cleanup:
    return exit_status;
}

static struct h3c_result *create_result_ross(uint16_t ross_id)
{
    struct h3c_result *result = 0;
    if (h3c_open(h3master_address(), 51371 + ross_id)) FAIL("h3c_open");

    FILE *file = 0;
    if (!(file = fopen(ASSETS "/ross.fasta", "r"))) FAIL("fopen");
    if (!(result = h3c_result_new())) FAIL("h3c_result_new")
    if (h3c_call(cmd, file, result)) FAIL("h3c_call");
    fclose(file);

    if (h3c_close()) FAIL("h3c_close");

cleanup:
    if (exit_status)
    {
        if (result)
        {
            h3c_result_del(result);
            result = 0;
        }
    }
    return result;
}

static int check_hash(char const *filepath, int64_t hash, char const *source,
                      int line)
{
    FILE *file = 0;
    int64_t h = 0;

    if (!(file = fopen(filepath, "rb"))) XFAIL("fopen", source, line);
    if (!file_hash(file, &h)) XFAIL("file_hash", source, line);
    printf("hash: %lld\n", h);
    if (h != hash) XFAIL("match file hash", source, line);
    fclose(file);

cleanup:
    return exit_status;
}

#define CHECK_HASH(F, H) check_hash(F, H, __FILE__, __LINE__)

static int test_pack_result(uint16_t ross_id)
{
    struct h3c_result *result = create_result_ross(ross_id);
    if (!result) goto cleanup;

    FILE *file = 0;
    if (!(file = fopen(TMPDIR "/h3result.mp", "wb"))) FAIL("fopen");
    if (h3c_result_pack(result, file)) FAIL("h3c_result_pack");
    fclose(file);

    if (CHECK_HASH(TMPDIR "/h3result.mp", -7580715214164890760LL)) goto cleanup;

cleanup:
    if (result) h3c_result_del(result);
    return exit_status;
}

static int test_print_targets(uint16_t ross_id)
{
    struct h3c_result *result = create_result_ross(ross_id);
    if (!result) goto cleanup;

    FILE *file = 0;
    if (!(file = fopen(TMPDIR "/targets.txt", "wb"))) FAIL("fopen");
    h3c_result_print_targets(result, file);
    fclose(file);

    if (CHECK_HASH(TMPDIR "/targets.txt", 2235430570033520642LL)) goto cleanup;

cleanup:
    if (result) h3c_result_del(result);
    return exit_status;
}

static int test_print_domains(uint16_t ross_id)
{
    struct h3c_result *result = create_result_ross(ross_id);
    if (!result) goto cleanup;

    FILE *file = 0;
    if (!(file = fopen(TMPDIR "/domains.txt", "wb"))) FAIL("fopen");
    h3c_result_print_domains(result, file);
    fclose(file);

    if (CHECK_HASH(TMPDIR "/domains.txt", 450185627565076573LL)) goto cleanup;

cleanup:
    if (result) h3c_result_del(result);
    return exit_status;
}

static int test_print_targets_table(uint16_t ross_id)
{
    struct h3c_result *result = create_result_ross(ross_id);
    if (!result) goto cleanup;

    FILE *file = 0;
    if (!(file = fopen(TMPDIR "/targets.tbl", "wb"))) FAIL("fopen");
    h3c_result_print_targets_table(result, file);
    fclose(file);

    if (CHECK_HASH(TMPDIR "/targets.tbl", -705996778582966846LL)) goto cleanup;

cleanup:
    if (result) h3c_result_del(result);
    return exit_status;
}

static int test_reuse_results(void)
{
    struct h3c_result *result = 0;
    FILE *file = 0;

    if (!(result = h3c_result_new())) FAIL("h3c_result_new")

    for (unsigned i = 0; i < 5; ++i)
    {
        if (!(file = fopen(ASSETS "/ross.fasta", "r"))) FAIL("fopen");
        if (h3c_open(h3master_address(), 51371 + i)) FAIL("h3c_open");
        if (h3c_call(cmd, file, result)) FAIL("h3c_call");
        if (h3c_close()) FAIL("h3c_close");
        fclose(file);
    }

cleanup:
    if (result) h3c_result_del(result);
    return exit_status;
}

static int test_reuse_results_print(void)
{
    int64_t targets[] = {3595942196364536930LL, -2350707101380469820LL,
                         7268596939732165182LL, 624009942923406464LL,
                         2235430570033520642LL};
    int64_t domains[] = {3742250844459566216LL, 6798122216300339939LL,
                         -6277080907676284547LL, -5934588947600765520LL,
                         450185627565076573LL};
    struct h3c_result *result = 0;
    FILE *file = 0;

    if (!(result = h3c_result_new())) FAIL("h3c_result_new")

    for (unsigned i = 0; i < 5; ++i)
    {
        if (!(file = fopen(ASSETS "/ross.fasta", "r"))) FAIL("fopen");
        if (h3c_open(h3master_address(), 51371 + i)) FAIL("h3c_open");
        if (h3c_call(cmd, file, result)) FAIL("h3c_call");
        if (h3c_close()) FAIL("h3c_close");
        fclose(file);

        file = 0;
        if (!(file = fopen(TMPDIR "/targets.txt", "wb"))) FAIL("fopen");
        h3c_result_print_targets(result, file);
        fclose(file);
        if (CHECK_HASH(TMPDIR "/targets.txt", targets[i])) goto cleanup;

        file = 0;
        if (!(file = fopen(TMPDIR "/domains.txt", "wb"))) FAIL("fopen");
        h3c_result_print_domains(result, file);
        fclose(file);
        if (CHECK_HASH(TMPDIR "/domains.txt", domains[i])) goto cleanup;
    }

cleanup:
    if (result) h3c_result_del(result);
    return exit_status;
}

static int test_reuse_connection(uint16_t ross_id)
{
    struct h3c_result *result = 0;
    bool connected = false;
    FILE *file = 0;

    if (!(result = h3c_result_new())) FAIL("h3c_result_new")
    if (h3c_open(h3master_address(), 51371 + ross_id)) FAIL("h3c_open");
    connected = true;

    if (!(file = fopen(ASSETS "/ross.fasta", "r"))) FAIL("fopen");
    if (h3c_call(cmd, file, result)) FAIL("h3c_call");
    fclose(file);

    if (!(file = fopen(ASSETS "/ross.poor.fasta", "r"))) FAIL("fopen");
    if (h3c_call(cmd, file, result)) FAIL("h3c_call");
    fclose(file);

    connected = false;
    if (h3c_close()) FAIL("h3c_close");

cleanup:
    if (connected) h3c_close();
    if (result) h3c_result_del(result);
    return exit_status;
}

int main(void)
{
    if (test_open_close_connection(4)) goto cleanup;
    if (test_pack_result(4)) goto cleanup;
    if (test_print_targets(4)) goto cleanup;
    if (test_print_domains(4)) goto cleanup;
    if (test_print_targets_table(4)) goto cleanup;
    if (test_reuse_results()) goto cleanup;
    if (test_reuse_results_print()) goto cleanup;
    if (test_reuse_connection(4)) goto cleanup;

cleanup:
    return exit_status;
}
