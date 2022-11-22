#include "file_hash.h"
#include "fs.h"
#include "h3c/h3c.h"
#include "helper.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define PORT 51379
#define ROSS_GOOD 0
#define ROSS_BAD 1
static char const *seqs[2] = {0};
static char const cmd[] = "--hmmdb 1 --acc --cut_ga";

static void test_connection_timedout(void);
static void test_open_close_connection(void);
static void test_request_timedout(void);
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

static void assets_setup(void);
static void assets_cleanup(void);

int main(void)
{
    atexit(h3c_fini);

    assets_setup();
    test_connection_timedout();
    test_open_close_connection();
    test_request_timedout();
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
    assets_cleanup();
    return EXIT_SUCCESS;
}

static long earlier(void) { return h3c_deadline(-1); }
static long later(void) { return h3c_deadline(1000 * 5); }

static bool same_hash(char const *filepath, long hash);
static struct h3c_dialer *conn_new(void);
static struct h3c_stream *conn_dial(struct h3c_dialer *d, long deadline);
static void conn_del(struct h3c_dialer *d);
static struct h3c_result *request(struct h3c_stream *s, char const *seq,
                                  long deadline);

static void test_connection_timedout(void)
{
    struct h3c_dialer *d = conn_new();
    if (h3c_dialer_dial(d, earlier()) != H3C_ETIMEDOUT) fail();
    conn_del(d);
}

static void test_open_close_connection(void)
{
    struct h3c_dialer *d = conn_new();
    check_code(h3c_dialer_dial(d, later()));
    h3c_dialer_del(d);
}

static void test_request_timedout(void)
{
    struct h3c_dialer *d = conn_new();
    struct h3c_stream *s = conn_dial(d, later());

    check_code(h3c_stream_put(s, cmd, seqs[ROSS_GOOD], earlier()));
    h3c_stream_wait(s);
    struct h3c_result *result = h3c_result_new();
    if (!result) fail();
    if (h3c_stream_pop(s, result) != H3C_ETIMEDOUT) fail();

    h3c_result_del(result);
    h3c_stream_del(s);
    conn_del(d);
}

static struct h3c_result *request_ross(void)
{
    struct h3c_dialer *d = conn_new();
    struct h3c_stream *s = conn_dial(d, later());
    struct h3c_result *r = request(s, seqs[ROSS_GOOD], later());

    h3c_stream_del(s);
    conn_del(d);
    return r;
}

static void test_pack_result(void)
{
    struct h3c_result *result = request_ross();
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
    struct h3c_result *result = request_ross();
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
    struct h3c_result *result = request_ross();
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
    struct h3c_result *result = request_ross();
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
    struct h3c_result *result = request_ross();
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
    struct h3c_result *result = request_ross();
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

    if (!(result = h3c_result_new())) fail();

    struct h3c_dialer *d = conn_new();
    check_code(h3c_dialer_dial(d, later()));

    struct h3c_stream *s = h3c_dialer_stream(d);

    check_code(h3c_stream_put(s, cmd, seqs[ROSS_GOOD], later()));
    h3c_stream_wait(s);
    check_code(h3c_stream_pop(s, result));

    h3c_stream_del(s);
    h3c_dialer_del(d);

    h3c_result_del(result);
}

static void test_reuse_results_print(void)
{
    int64_t target = 57543L;
    int64_t domain = 46469L;
    struct h3c_result *result = 0;
    FILE *file = 0;

    if (!(result = h3c_result_new())) fail();

    struct h3c_dialer *d = conn_new();
    check_code(h3c_dialer_dial(d, later()));

    struct h3c_stream *s = h3c_dialer_stream(d);

    check_code(h3c_stream_put(s, cmd, seqs[ROSS_GOOD], later()));
    h3c_stream_wait(s);
    check_code(h3c_stream_pop(s, result));

    h3c_stream_del(s);
    h3c_dialer_del(d);

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

    if (!(result = h3c_result_new())) fail();

    struct h3c_dialer *d = conn_new();
    check_code(h3c_dialer_dial(d, later()));

    struct h3c_stream *s = h3c_dialer_stream(d);

    check_code(h3c_stream_put(s, cmd, seqs[ROSS_GOOD], later()));
    h3c_stream_wait(s);
    check_code(h3c_stream_pop(s, result));

    check_code(h3c_stream_put(s, cmd, seqs[ROSS_BAD], later()));
    h3c_stream_wait(s);
    check_code(h3c_stream_pop(s, result));

    h3c_stream_del(s);
    h3c_dialer_del(d);

    h3c_result_del(result);
}

static void test_result_api(void)
{
    struct h3c_result *result = request_ross();
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

static void assets_setup(void)
{
    long size = 0;
    unsigned char *data = NULL;

    check_code(fs_readall(ASSETS "/ross.fasta", &size, &data));
    seqs[ROSS_GOOD] = append_char(size, (char *)data, '\0');

    check_code(fs_readall(ASSETS "/ross.poor.fasta", &size, &data));
    seqs[ROSS_BAD] = append_char(size, (char *)data, '\0');
}

static void assets_cleanup(void)
{
    for (size_t i = 0; i < array_size(seqs); ++i)
        free((void *)seqs[i]);
}

static bool same_hash(char const *filepath, long hash)
{
    long expected = 0;
    if (!file_hash(filepath, &expected)) fail();
    return expected == hash;
}

static struct h3c_dialer *conn_new(void)
{
    struct h3c_dialer *d = h3c_dialer_new("127.0.0.1", PORT);
    if (!d) fail();
    return d;
}

static struct h3c_stream *conn_dial(struct h3c_dialer *d, long deadline)
{
    check_code(h3c_dialer_dial(d, deadline));
    return h3c_dialer_stream(d);
}

static void conn_del(struct h3c_dialer *d) { h3c_dialer_del(d); }

static struct h3c_result *request(struct h3c_stream *s, char const *seq,
                                  long deadline)
{
    check_code(h3c_stream_put(s, cmd, seq, deadline));
    h3c_stream_wait(s);
    struct h3c_result *result = h3c_result_new();
    if (!result) fail();
    check_code(h3c_stream_pop(s, result));
    return result;
}
