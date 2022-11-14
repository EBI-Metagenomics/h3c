#include "h3c/h3c.h"
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

static long deadline(void) { return h3c_now() + 1000 * 5; }

#define ARRAY_SIZE(arr) (sizeof(arr) / sizeof((arr)[0]))
#define PORT 51379

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

static int test_open_close_connection(void)
{
    if (h3c_open("127.0.0.1", PORT, deadline())) FAIL("h3c_open");
    if (h3c_close(deadline())) FAIL("h3c_close");

cleanup:
    return exit_status;
}

static struct h3c_result *create_result_ross(void)
{
    struct h3c_result *result = 0;
    if (h3c_open("127.0.0.1", PORT, deadline())) FAIL("h3c_open");

    FILE *file = 0;
    if (!(file = fopen(ASSETS "/ross.fasta", "r"))) FAIL("fopen");
    if (!(result = h3c_result_new())) FAIL("h3c_result_new")
    if (h3c_send(cmd, file, result, deadline())) FAIL("h3c_callf");
    fclose(file);

    if (h3c_close(deadline())) FAIL("h3c_close");

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

static int check(bool value, char const *source, int line)
{
    if (!value) XFAIL("check failed", source, line);

cleanup:
    return exit_status;
}

static int check_hash(char const *filepath, long hash, char const *source,
                      int line)
{
    long expected = 0;
    if (!file_hash(filepath, &expected)) XFAIL("file_hash", source, line);
    printf("%ld -> %ld\n", hash, expected);
    if (expected != hash) XFAIL("match file hash", source, line);

cleanup:
    return exit_status;
}

static double fabs(double x) { return x < 0 ? -x : x; }

static bool is_close(double a, double b) { return fabs(a - b) < 1e-7; }

#define CHECK(V) check(V, __FILE__, __LINE__)
#define CLOSE(A, B) check(is_close(A, B), __FILE__, __LINE__)
#define STREQ(A, B) check(!strcmp(A, B), __FILE__, __LINE__)
#define CHECK_HASH(F, H) check_hash(F, H, __FILE__, __LINE__)

static int test_pack_result(void)
{
    struct h3c_result *result = create_result_ross();
    if (!result) goto cleanup;

    FILE *file = 0;
    if (!(file = fopen(TMPDIR "/h3result.mp", "wb"))) FAIL("fopen");
    if (h3c_result_pack(result, file)) FAIL("h3c_result_pack");
    fclose(file);

    if (CHECK_HASH(TMPDIR "/h3result.mp", 63792L)) goto cleanup;

cleanup:
    if (result) h3c_result_del(result);
    return exit_status;
}

static int test_unpack_result(void)
{
    struct h3c_result *result = create_result_ross();
    if (!result) goto cleanup;

    FILE *file = 0;
    if (!(file = fopen(TMPDIR "/h3result.mp", "wb"))) FAIL("fopen");
    if (h3c_result_pack(result, file)) FAIL("h3c_result_pack");
    fclose(file);

    if (CHECK_HASH(TMPDIR "/h3result.mp", 63792L)) goto cleanup;

    if (!(file = fopen(TMPDIR "/h3result.mp", "rb"))) FAIL("fopen");
    if (h3c_result_unpack(result, file)) FAIL("h3c_result_unpack");
    fclose(file);

    file = 0;
    if (!(file = fopen(TMPDIR "/h3result.mp", "wb"))) FAIL("fopen");
    if (h3c_result_pack(result, file)) FAIL("h3c_result_pack");
    fclose(file);

    if (CHECK_HASH(TMPDIR "/h3result.mp", 63792L)) goto cleanup;

cleanup:
    if (result) h3c_result_del(result);
    return exit_status;
}

static int test_print_targets(void)
{
    struct h3c_result *result = create_result_ross();
    if (!result) goto cleanup;

    FILE *file = 0;
    if (!(file = fopen(TMPDIR "/targets.txt", "wb"))) FAIL("fopen");
    h3c_result_print_targets(result, file);
    fclose(file);

    if (CHECK_HASH(TMPDIR "/targets.txt", 57543L)) goto cleanup;

cleanup:
    if (result) h3c_result_del(result);
    return exit_status;
}

static int test_print_domains(void)
{
    struct h3c_result *result = create_result_ross();
    if (!result) goto cleanup;

    FILE *file = 0;
    if (!(file = fopen(TMPDIR "/domains.txt", "wb"))) FAIL("fopen");
    h3c_result_print_domains(result, file);
    fclose(file);

    if (CHECK_HASH(TMPDIR "/domains.txt", 46469L)) goto cleanup;

cleanup:
    if (result) h3c_result_del(result);
    return exit_status;
}

static int test_print_targets_table(void)
{
    struct h3c_result *result = create_result_ross();
    if (!result) goto cleanup;

    FILE *file = 0;
    if (!(file = fopen(TMPDIR "/targets.tbl", "wb"))) FAIL("fopen");
    h3c_result_print_targets_table(result, file);
    fclose(file);

    if (CHECK_HASH(TMPDIR "/targets.tbl", 34790L)) goto cleanup;

cleanup:
    if (result) h3c_result_del(result);
    return exit_status;
}

static int test_print_domains_table(void)
{
    struct h3c_result *result = create_result_ross();
    if (!result) goto cleanup;

    FILE *file = 0;
    if (!(file = fopen(TMPDIR "/domains.tbl", "wb"))) FAIL("fopen");
    h3c_result_print_domains_table(result, file);
    fclose(file);

    if (CHECK_HASH(TMPDIR "/domains.tbl", 3913L)) goto cleanup;

cleanup:
    if (result) h3c_result_del(result);
    return exit_status;
}

static int test_reuse_results(void)
{
    struct h3c_result *result = 0;
    FILE *file = 0;

    if (!(result = h3c_result_new())) FAIL("h3c_result_new")

    if (!(file = fopen(ASSETS "/ross.fasta", "r"))) FAIL("fopen");
    if (h3c_open("127.0.0.1", PORT, deadline())) FAIL("h3c_open");
    if (h3c_send(cmd, file, result, deadline())) FAIL("h3c_callf");
    if (h3c_close(deadline())) FAIL("h3c_close");
    fclose(file);

cleanup:
    if (result) h3c_result_del(result);
    return exit_status;
}

static int test_reuse_results_print(void)
{
    int64_t target = 57543L;
    int64_t domain = 46469L;
    struct h3c_result *result = 0;
    FILE *file = 0;

    if (!(result = h3c_result_new())) FAIL("h3c_result_new")

    if (!(file = fopen(ASSETS "/ross.fasta", "r"))) FAIL("fopen");
    if (h3c_open("127.0.0.1", PORT, deadline())) FAIL("h3c_open");
    if (h3c_send(cmd, file, result, deadline())) FAIL("h3c_callf");
    if (h3c_close(deadline())) FAIL("h3c_close");
    fclose(file);

    file = 0;
    if (!(file = fopen(TMPDIR "/targets.txt", "wb"))) FAIL("fopen");
    h3c_result_print_targets(result, file);
    fclose(file);
    if (CHECK_HASH(TMPDIR "/targets.txt", target)) goto cleanup;

    file = 0;
    if (!(file = fopen(TMPDIR "/domains.txt", "wb"))) FAIL("fopen");
    h3c_result_print_domains(result, file);
    fclose(file);
    if (CHECK_HASH(TMPDIR "/domains.txt", domain)) goto cleanup;

cleanup:
    if (result) h3c_result_del(result);
    return exit_status;
}

static int test_reuse_connection(void)
{
    struct h3c_result *result = 0;
    bool connected = false;
    FILE *file = 0;

    if (!(result = h3c_result_new())) FAIL("h3c_result_new")
    if (h3c_open("127.0.0.1", PORT, deadline())) FAIL("h3c_open");
    connected = true;

    if (!(file = fopen(ASSETS "/ross.fasta", "r"))) FAIL("fopen");
    if (h3c_send(cmd, file, result, deadline())) FAIL("h3c_callf");
    fclose(file);

    if (!(file = fopen(ASSETS "/ross.poor.fasta", "r"))) FAIL("fopen");
    if (h3c_send(cmd, file, result, deadline())) FAIL("h3c_callf");
    fclose(file);

    connected = false;
    if (h3c_close(deadline())) FAIL("h3c_close");

cleanup:
    if (connected) h3c_close(deadline());
    if (result) h3c_result_del(result);
    return exit_status;
}

static int test_result_api(void)
{
    struct h3c_result *result = create_result_ross();
    if (!result) goto cleanup;

    char const *name[] = {"000000005", "000000003", "000000002", "000000004"};
    char const *acc[] = {"PF13460.8", "PF01370.23", "PF01073.21", "PF05368.15"};
    double eln[] = {-53.808984215028, -38.604817925966, -34.047928969184,
                    -31.743561848164};

    if (CHECK(h3c_result_nhits(result) == 4)) goto cleanup;

    for (int i = 0; i < 4; ++i)
    {
        if (STREQ(name[i], h3c_result_hit_name(result, i))) goto cleanup;
        if (STREQ(acc[i], h3c_result_hit_acc(result, i))) goto cleanup;
        if (CLOSE(eln[i], h3c_result_hit_evalue_ln(result, i))) goto cleanup;
    }

cleanup:
    if (result) h3c_result_del(result);
    return exit_status;
}

int main(void)
{
    if (test_open_close_connection()) goto cleanup;
    if (test_pack_result()) goto cleanup;
    if (test_unpack_result()) goto cleanup;
    if (test_print_targets()) goto cleanup;
    if (test_print_domains()) goto cleanup;
    if (test_print_targets_table()) goto cleanup;
    if (test_print_domains_table()) goto cleanup;
    if (test_reuse_results()) goto cleanup;
    if (test_reuse_results_print()) goto cleanup;
    if (test_reuse_connection()) goto cleanup;
    if (test_result_api()) goto cleanup;

cleanup:
    return exit_status;
}
