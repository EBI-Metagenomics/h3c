#include "hmmd/stats.h"
#include "c_toolbelt/c_toolbelt.h"
#include "h3client/h3client.h"
#include "hmmd/utils.h"
#include "hmmd/zsetby.h"
#include <assert.h>
#include <stdlib.h>
#include <string.h>

void hmmd_stats_init(struct hmmd_stats *stats)
{
    memset(stats, 0, sizeof(*stats));
}

void hmmd_stats_cleanup(struct hmmd_stats *stats)
{
    free(stats->hit_offsets);
    stats->hit_offsets = 0;
}

static enum h3c_rc parse_first_part(struct hmmd_stats *stats,
                                    unsigned char const **ptr,
                                    unsigned char const *end)
{
    enum h3c_rc rc = H3C_OK;

    ESCAPE_OVERRUN(rc, *ptr, end, 14 * sizeof(uint64_t) + 2);

    // skip elapsed
    (void)eatf64(ptr);
    // skip user
    (void)eatf64(ptr);
    // skip sys
    (void)eatf64(ptr);

    stats->Z = eatf64(ptr);
    stats->domZ = eatf64(ptr);

    if ((rc = hmmd_zsetby_parse(&stats->Z_setby, ptr))) goto cleanup;
    if ((rc = hmmd_zsetby_parse(&stats->domZ_setby, ptr))) goto cleanup;

    stats->nmodels = eatu64(ptr);
    stats->nseqs = eatu64(ptr);
    stats->n_past_msv = eatu64(ptr);
    stats->n_past_bias = eatu64(ptr);
    stats->n_past_vit = eatu64(ptr);
    stats->n_past_fwd = eatu64(ptr);
    stats->nhits = eatu64(ptr);
    stats->nreported = eatu64(ptr);
    stats->nincluded = eatu64(ptr);

cleanup:
    return rc;
}

enum h3c_rc hmmd_stats_parse(struct hmmd_stats *stats,
                             unsigned char const **ptr,
                             unsigned char const *end)
{
    enum h3c_rc rc = H3C_OK;

    if ((rc = parse_first_part(stats, ptr, end))) goto cleanup;

    ESCAPE_OVERRUN(rc, *ptr, end, sizeof(uint64_t));
    uint64_t hit_offset = eatu64(ptr);

    if (hit_offset == UINT64_MAX && stats->nhits > 0)
    {
        rc = H3C_FAILED_PARSE;
        goto cleanup;
    }

    if (hit_offset != UINT64_MAX)
    {
        size_t size = stats->nhits * sizeof(uint64_t);
        stats->hit_offsets = ctb_realloc(stats->hit_offsets, size);
        if (!stats->hit_offsets)
        {
            rc = H3C_NOT_ENOUGH_MEMORY;
            goto cleanup;
        }

        ESCAPE_OVERRUN(rc, *ptr, end, stats->nhits * sizeof(uint64_t));
        stats->hit_offsets[0] = hit_offset;
        for (uint64_t i = 1; i < stats->nhits; i++)
            stats->hit_offsets[i] = eatu64(ptr);
    }

    return H3C_OK;

cleanup:
    hmmd_stats_cleanup(stats);
    return rc;
}
