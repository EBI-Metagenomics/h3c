#include "h3c/result.h"
#include "h3c/errno.h"
#include "lip/lip.h"
#include "result.h"
#include "utils.h"
#include <stdlib.h>

struct h3c_result *h3c_result_new(void)
{
    struct h3c_result *result = malloc(sizeof(*result));
    if (!result) return 0;
    result->errnum = 0;
    result->errstr = NULL;
    h3c_stats_init(&result->stats);
    h3c_tophits_init(&result->tophits);
    return result;
}

void h3c_result_del(struct h3c_result const *result)
{
    if (result->errstr) free((void *)result->errstr);
    h3c_tophits_cleanup((struct tophits *)&result->tophits);
    free((void *)result);
}

int h3c_result_pack(struct h3c_result const *result, FILE *file)
{
    struct lip_file f = {0};
    lip_file_init(&f, file);

    lip_write_map_size(&f, 1);
    lip_write_cstr(&f, "h3result");

    lip_write_map_size(&f, 2);
    lip_write_cstr(&f, "stats");
    int rc = h3c_stats_pack(&result->stats, &f);
    if (rc) return rc;

    lip_write_cstr(&f, "tophits");
    return h3c_tophits_pack(&result->tophits, &f);
}

int h3c_result_unpack(struct h3c_result *result, FILE *file)
{
    struct lip_file f = {0};
    lip_file_init(&f, file);

    if (!h3c_expect_map_size(&f, 1)) return H3C_EUNPACK;

    if (!h3c_expect_key(&f, "h3result")) return H3C_EUNPACK;

    if (!h3c_expect_map_size(&f, 2)) return H3C_EUNPACK;
    if (!h3c_expect_key(&f, "stats")) return H3C_EUNPACK;
    int rc = h3c_stats_unpack(&result->stats, &f);
    if (rc) return rc;

    if (!h3c_expect_key(&f, "tophits")) return H3C_EUNPACK;
    return h3c_tophits_unpack(&result->tophits, &f);
}

int h3c_result_errnum(struct h3c_result const *x) { return x->errnum; }

char const *h3c_result_errstr(struct h3c_result const *x) { return x->errstr; }

void h3c_result_print_targets(struct h3c_result const *r, FILE *file)
{
    h3c_tophits_print_targets(&r->tophits, file, r->stats.Z);
}

void h3c_result_print_domains(struct h3c_result const *r, FILE *file)
{
    h3c_tophits_print_domains(&r->tophits, file, r->stats.Z, r->stats.domZ);
}

void h3c_result_print_targets_table(struct h3c_result const *r, FILE *file)
{
    h3c_tophits_print_targets_table("-", &r->tophits, file, true, r->stats.Z);
}

void h3c_result_print_domains_table(struct h3c_result const *r, FILE *file)
{
    h3c_tophits_print_domains_table("-", &r->tophits, file, true, r->stats.Z,
                                    r->stats.domZ);
}

unsigned h3c_result_nhits(struct h3c_result const *r)
{
    return r->tophits.nhits;
}

char const *h3c_result_hit_name(struct h3c_result const *r, unsigned idx)
{
    return h3c_tophits_hit_name(&r->tophits, idx);
}

char const *h3c_result_hit_acc(struct h3c_result const *r, unsigned idx)
{
    return h3c_tophits_hit_acc(&r->tophits, idx);
}

double h3c_result_hit_evalue_ln(struct h3c_result const *r, unsigned idx)
{
    return h3c_tophits_hit_evalue_ln(&r->tophits, idx, r->stats.Z);
}
