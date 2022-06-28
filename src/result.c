#include "h3client/result.h"
#include "h3client/rc.h"
#include "lite_pack/lite_pack.h"
#include "result.h"
#include <stdlib.h>

struct h3c_result *h3c_result_new(void)
{
    struct h3c_result *result = malloc(sizeof(*result));
    if (!result) return 0;
    stats_init(&result->stats);
    tophits_init(&result->tophits);
    return result;
}

void h3c_result_del(struct h3c_result const *result)
{
    tophits_cleanup((struct h3c_tophits *)&result->tophits);
    free((void *)result);
}

enum h3c_rc h3c_result_pack(struct h3c_result const *result, FILE *file)
{
    enum h3c_rc rc = H3C_OK;

    struct lip_file f = {0};
    lip_file_init(&f, file);

    lip_write_map_size(&f, 1);
    lip_write_cstr(&f, "h3result");

    lip_write_map_size(&f, 2);
    lip_write_cstr(&f, "stats");
    if (lip_file_error(&f)) return H3C_FAILED_PACK;

    if ((rc = stats_pack(&result->stats, &f))) return rc;

    lip_write_cstr(&f, "tophits");
    if (lip_file_error(&f)) return H3C_FAILED_PACK;

    return tophits_pack(&result->tophits, &f);
}

void h3c_result_print_targets(struct h3c_result const *result, FILE *file)
{
    tophits_print_targets(&result->tophits, file, result->stats.Z);
}

void h3c_result_print_domains(struct h3c_result const *result, FILE *file)
{
    tophits_print_domains(&result->tophits, file, result->stats.Z,
                          result->stats.domZ);
}

#if 0
void answer_print(struct answer const *ans)
{
    hmmd_tophits_print_targets(&ans->tophits, true, ans->stats.Z);
    hmmd_tophits_print_domains(&ans->tophits, true, ans->stats.Z,
                               ans->stats.domZ, true);

    hmmd_tophits_print_tabular_targets("QNAME", "QACC", &ans->tophits, true,
                                       ans->stats.Z);

    hmmd_tophits_print_tabular_domains("QNAME", "QACC", &ans->tophits, true,
                                       ans->stats.Z, ans->stats.domZ);
}
#endif
