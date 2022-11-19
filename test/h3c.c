#include "h3c/h3c.h"
#include "file_hash.h"
#include "helper.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define PORT 51379
static char const cmd[] = "--hmmdb 1 --acc --cut_ga";

static void test_open_close_connection(void);
static void test_pack_result(void);
static void test_unpack_result(void);
static void test_print_targets(void);
static void test_print_domains(void);
static void test_print_targets_table(void);
static void test_print_domains_table(void);
static void test_reuse_results(void);
static void test_reuse_results_print(void);
static void test_reuse_connection(void);
static void test_result_api(void);

int main(void)
{
    test_open_close_connection();
    test_pack_result();
    test_unpack_result();
    test_print_targets();
    test_print_domains();
    test_print_targets_table();
    test_print_domains_table();
    test_reuse_results();
    test_reuse_results_print();
    test_reuse_connection();
    test_result_api();
    return EXIT_SUCCESS;
}

static long deadline(void) { return h3c_now() + 1000 * 500; }

static void test_open_close_connection(void)
{
    check_code(h3c_open("127.0.0.1", PORT, deadline()));
    h3c_close();
}

static struct h3c_result *create_result_ross(void)
{
    struct h3c_result *result = 0;
    check_code(h3c_open("127.0.0.1", PORT, deadline()));

    FILE *file = 0;
    if (!(file = fopen(ASSETS "/ross.fasta", "r"))) fail();
    if (!(result = h3c_result_new())) fail();
    check_code(h3c_send(cmd, file, result, deadline()));
    fclose(file);

    h3c_close();
    return result;
}

static bool same_hash(char const *filepath, long hash)
{
    long expected = 0;
    if (!file_hash(filepath, &expected)) fail();
    return expected == hash;
}

static void test_pack_result(void)
{
    struct h3c_result *result = create_result_ross();
    if (!result) fail();

    FILE *file = 0;
    if (!(file = fopen(TMPDIR "/h3result.mp", "wb"))) fail();
    check_code(h3c_result_pack(result, file));
    fclose(file);

    if (!same_hash(TMPDIR "/h3result.mp", 63792L)) fail();

    h3c_result_del(result);
}

static void test_unpack_result(void)
{
    struct h3c_result *result = create_result_ross();
    if (!result) fail();

    FILE *file = 0;
    if (!(file = fopen(TMPDIR "/h3result.mp", "wb"))) fail();
    check_code(h3c_result_pack(result, file));
    fclose(file);

    if (!same_hash(TMPDIR "/h3result.mp", 63792L)) fail();

    if (!(file = fopen(TMPDIR "/h3result.mp", "rb"))) fail();
    check_code(h3c_result_unpack(result, file));
    fclose(file);

    file = 0;
    if (!(file = fopen(TMPDIR "/h3result.mp", "wb"))) fail();
    check_code(h3c_result_pack(result, file));
    fclose(file);

    if (!same_hash(TMPDIR "/h3result.mp", 63792L)) fail();

    h3c_result_del(result);
}

static void test_print_targets(void)
{
    struct h3c_result *result = create_result_ross();
    if (!result) fail();

    FILE *file = 0;
    if (!(file = fopen(TMPDIR "/targets.txt", "wb"))) fail();
    h3c_result_print_targets(result, file);
    fclose(file);

    if (!same_hash(TMPDIR "/targets.txt", 57543L)) fail();

    h3c_result_del(result);
}

static void test_print_domains(void)
{
    struct h3c_result *result = create_result_ross();
    if (!result) fail();

    FILE *file = 0;
    if (!(file = fopen(TMPDIR "/domains.txt", "wb"))) fail();
    h3c_result_print_domains(result, file);
    fclose(file);

    if (!same_hash(TMPDIR "/domains.txt", 46469L)) fail();

    h3c_result_del(result);
}

static void test_print_targets_table(void)
{
    struct h3c_result *result = create_result_ross();
    if (!result) fail();

    FILE *file = 0;
    if (!(file = fopen(TMPDIR "/targets.tbl", "wb"))) fail();
    h3c_result_print_targets_table(result, file);
    fclose(file);

    if (!same_hash(TMPDIR "/targets.tbl", 34790L)) fail();

    h3c_result_del(result);
}

static void test_print_domains_table(void)
{
    struct h3c_result *result = create_result_ross();
    if (!result) fail();

    FILE *file = 0;
    if (!(file = fopen(TMPDIR "/domains.tbl", "wb"))) fail();
    h3c_result_print_domains_table(result, file);
    fclose(file);

    if (!same_hash(TMPDIR "/domains.tbl", 3913L)) fail();

    h3c_result_del(result);
}

static void test_reuse_results(void)
{
    struct h3c_result *result = 0;
    FILE *file = 0;

    if (!(result = h3c_result_new())) fail();

    if (!(file = fopen(ASSETS "/ross.fasta", "r"))) fail();
    check_code(h3c_open("127.0.0.1", PORT, deadline()));
    check_code(h3c_send(cmd, file, result, deadline()));
    h3c_close();
    fclose(file);

    h3c_result_del(result);
}

static void test_reuse_results_print(void)
{
    int64_t target = 57543L;
    int64_t domain = 46469L;
    struct h3c_result *result = 0;
    FILE *file = 0;

    if (!(result = h3c_result_new())) fail();

    if (!(file = fopen(ASSETS "/ross.fasta", "r"))) fail();
    check_code(h3c_open("127.0.0.1", PORT, deadline()));
    check_code(h3c_send(cmd, file, result, deadline()));
    h3c_close();
    fclose(file);

    file = 0;
    if (!(file = fopen(TMPDIR "/targets.txt", "wb"))) fail();
    h3c_result_print_targets(result, file);
    fclose(file);
    if (!same_hash(TMPDIR "/targets.txt", target)) fail();

    file = 0;
    if (!(file = fopen(TMPDIR "/domains.txt", "wb"))) fail();
    h3c_result_print_domains(result, file);
    fclose(file);
    if (!same_hash(TMPDIR "/domains.txt", domain)) fail();

    h3c_result_del(result);
}

static void test_reuse_connection(void)
{
    struct h3c_result *result = 0;
    FILE *file = 0;

    if (!(result = h3c_result_new())) fail();
    check_code(h3c_open("127.0.0.1", PORT, deadline()));

    if (!(file = fopen(ASSETS "/ross.fasta", "r"))) fail();
    check_code(h3c_send(cmd, file, result, deadline()));
    fclose(file);

    if (!(file = fopen(ASSETS "/ross.poor.fasta", "r"))) fail();
    check_code(h3c_send(cmd, file, result, deadline()));
    fclose(file);

    h3c_close();

    h3c_result_del(result);
}

static void test_result_api(void)
{
    struct h3c_result *result = create_result_ross();
    if (!result) fail();

    char const *name[] = {"000000005", "000000003", "000000002", "000000004"};
    char const *acc[] = {"PF13460.8", "PF01370.23", "PF01073.21", "PF05368.15"};
    double eln[] = {-53.808984215028, -38.604817925966, -34.047928969184,
                    -31.743561848164};

    if (h3c_result_nhits(result) != 4) fail();

    for (int i = 0; i < 4; ++i)
    {
        if (strcmp(name[i], h3c_result_hit_name(result, i))) fail();
        if (strcmp(acc[i], h3c_result_hit_acc(result, i))) fail();
        if (!is_close(eln[i], h3c_result_hit_evalue_ln(result, i))) fail();
    }

    h3c_result_del(result);
}
