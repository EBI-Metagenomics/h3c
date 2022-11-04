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
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

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
    if (h3c_open("127.0.0.1", PORT)) FAIL("h3c_open");
    if (h3c_close()) FAIL("h3c_close");

cleanup:
    return exit_status;
}

static struct h3c_result *create_result_ross(void)
{
    struct h3c_result *result = 0;
    if (h3c_open("127.0.0.1", PORT)) FAIL("h3c_open");

    FILE *file = 0;
    if (!(file = fopen(ASSETS "/ross.fasta", "r"))) FAIL("fopen");
    if (!(result = h3c_result_new())) FAIL("h3c_result_new")
    if (h3c_callf(cmd, file, result)) FAIL("h3c_callf");
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

static int check(bool value, char const *source, int line)
{
    if (!value) XFAIL("check failed", source, line);

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

    if (CHECK_HASH(TMPDIR "/h3result.mp", -7580715214164890760LL)) goto cleanup;

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

    if (CHECK_HASH(TMPDIR "/h3result.mp", -7580715214164890760LL)) goto cleanup;

    if (!(file = fopen(TMPDIR "/h3result.mp", "rb"))) FAIL("fopen");
    if (h3c_result_unpack(result, file)) FAIL("h3c_result_unpack");
    fclose(file);

    file = 0;
    if (!(file = fopen(TMPDIR "/h3result.mp", "wb"))) FAIL("fopen");
    if (h3c_result_pack(result, file)) FAIL("h3c_result_pack");
    fclose(file);

    if (CHECK_HASH(TMPDIR "/h3result.mp", -7580715214164890760LL)) goto cleanup;

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

    if (CHECK_HASH(TMPDIR "/targets.txt", 2235430570033520642LL)) goto cleanup;

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

    if (CHECK_HASH(TMPDIR "/domains.txt", 450185627565076573LL)) goto cleanup;

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

    if (CHECK_HASH(TMPDIR "/targets.tbl", -705996778582966846LL)) goto cleanup;

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

    if (CHECK_HASH(TMPDIR "/domains.tbl", -1168549464075086691LL)) goto cleanup;

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
    if (h3c_open("127.0.0.1", PORT)) FAIL("h3c_open");
    if (h3c_callf(cmd, file, result)) FAIL("h3c_callf");
    if (h3c_close()) FAIL("h3c_close");
    fclose(file);

cleanup:
    if (result) h3c_result_del(result);
    return exit_status;
}

static int test_reuse_results_print(void)
{
    int64_t target = 2235430570033520642LL;
    int64_t domain = 450185627565076573LL;
    struct h3c_result *result = 0;
    FILE *file = 0;

    if (!(result = h3c_result_new())) FAIL("h3c_result_new")

    if (!(file = fopen(ASSETS "/ross.fasta", "r"))) FAIL("fopen");
    if (h3c_open("127.0.0.1", PORT)) FAIL("h3c_open");
    if (h3c_callf(cmd, file, result)) FAIL("h3c_callf");
    if (h3c_close()) FAIL("h3c_close");
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
    if (h3c_open("127.0.0.1", PORT)) FAIL("h3c_open");
    connected = true;

    if (!(file = fopen(ASSETS "/ross.fasta", "r"))) FAIL("fopen");
    if (h3c_callf(cmd, file, result)) FAIL("h3c_callf");
    fclose(file);

    if (!(file = fopen(ASSETS "/ross.poor.fasta", "r"))) FAIL("fopen");
    if (h3c_callf(cmd, file, result)) FAIL("h3c_callf");
    fclose(file);

    connected = false;
    if (h3c_close()) FAIL("h3c_close");

cleanup:
    if (connected) h3c_close();
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
