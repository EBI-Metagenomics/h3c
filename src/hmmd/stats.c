#include "hmmd/stats.h"
#include "h3c/h3c.h"
#include "hmmd/zsetby.h"
#include "utils.h"
#include "zc.h"
#include <assert.h>
#include <stdlib.h>
#include <string.h>

void h3c_hmmd_stats_init(struct hmmd_stats *stats)
{
    memset(stats, 0, sizeof(*stats));
}

void h3c_hmmd_stats_cleanup(struct hmmd_stats *stats)
{
    free(stats->hit_offsets);
    stats->hit_offsets = 0;
}

static int parse_first_part(struct hmmd_stats *stats, unsigned char const **ptr,
                            unsigned char const *end)
{
    int rc = H3C_OK;

    ESCAPE_OVERRUN(rc, *ptr, end, 14 * sizeof(uint64_t) + 2);

    // skip elapsed
    (void)h3c_eatf64(ptr);
    // skip user
    (void)h3c_eatf64(ptr);
    // skip sys
    (void)h3c_eatf64(ptr);

    stats->Z = h3c_eatf64(ptr);
    stats->domZ = h3c_eatf64(ptr);

    if ((rc = h3c_hmmd_zsetby_parse(&stats->Z_setby, ptr))) goto cleanup;
    if ((rc = h3c_hmmd_zsetby_parse(&stats->domZ_setby, ptr))) goto cleanup;

    stats->nmodels = h3c_eatu64(ptr);
    stats->nseqs = h3c_eatu64(ptr);
    stats->n_past_msv = h3c_eatu64(ptr);
    stats->n_past_bias = h3c_eatu64(ptr);
    stats->n_past_vit = h3c_eatu64(ptr);
    stats->n_past_fwd = h3c_eatu64(ptr);
    stats->nhits = h3c_eatu64(ptr);
    stats->nreported = h3c_eatu64(ptr);
    stats->nincluded = h3c_eatu64(ptr);

cleanup:
    return rc;
}

int h3c_hmmd_stats_parse(struct hmmd_stats *stats, unsigned char const **ptr,
                         unsigned char const *end)
{
    int rc = H3C_OK;

    if ((rc = parse_first_part(stats, ptr, end))) goto cleanup;

    ESCAPE_OVERRUN(rc, *ptr, end, sizeof(uint64_t));
    uint64_t hit_offset = h3c_eatu64(ptr);

    if (hit_offset == UINT64_MAX && stats->nhits > 0)
    {
        rc = H3C_EPARSE;
        goto cleanup;
    }

    size_t size = stats->nhits * sizeof(uint64_t);
    if (hit_offset == UINT64_MAX || size == 0) return H3C_OK;

    stats->hit_offsets = zc_reallocf(stats->hit_offsets, size);
    if (!stats->hit_offsets)
    {
        rc = H3C_ENOMEM;
        goto cleanup;
    }

    ESCAPE_OVERRUN(rc, *ptr, end, stats->nhits * sizeof(uint64_t));
    stats->hit_offsets[0] = hit_offset;
    for (uint64_t i = 1; i < stats->nhits; i++)
        stats->hit_offsets[i] = h3c_eatu64(ptr);

    return H3C_OK;

cleanup:
    h3c_hmmd_stats_cleanup(stats);
    return rc;
}
