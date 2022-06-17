#include "hmmd/stats.h"
#include "c_toolbelt/c_toolbelt.h"
#include "h3client/h3client.h"
#include "hmmd/utils.h"
#include "hmmd/zsetby.h"
#include <stdlib.h>
#include <string.h>

void hmmd_stats_init(struct hmmd_stats *stats)
{
    stats->elapsed = 0;
    memset(stats, 0, sizeof(*stats));
}

void hmmd_stats_cleanup(struct hmmd_stats *stats) { free(stats->hit_offsets); }

enum h3c_rc hmmd_stats_unpack(struct hmmd_stats *stats, size_t *read_size,
                              unsigned char const *data)
{
    enum h3c_rc rc = H3C_OK;
    *read_size = 0;
    unsigned char const *ptr = data;

    to_double(&stats->elapsed, eat64(&ptr));
    to_double(&stats->user, eat64(&ptr));
    to_double(&stats->sys, eat64(&ptr));
    to_double(&stats->Z, eat64(&ptr));
    to_double(&stats->domZ, eat64(&ptr));

    if ((rc = hmmd_zsetby_unpack(&stats->Z_setby, &ptr))) goto cleanup;
    if ((rc = hmmd_zsetby_unpack(&stats->domZ_setby, &ptr))) goto cleanup;

    stats->nmodels = eat64(&ptr);
    stats->nseqs = eat64(&ptr);
    stats->n_past_msv = eat64(&ptr);
    stats->n_past_bias = eat64(&ptr);
    stats->n_past_vit = eat64(&ptr);
    stats->n_past_fwd = eat64(&ptr);
    stats->nhits = eat64(&ptr);
    stats->nreported = eat64(&ptr);
    stats->nincluded = eat64(&ptr);

    uint64_t hit_offset = eat64(&ptr);
    if (hit_offset != UINT64_MAX)
    {
        stats->hit_offsets =
            ctb_realloc(stats->hit_offsets, stats->nhits * sizeof(uint64_t));
        if (!stats->hit_offsets)
        {
            rc = H3C_NOT_ENOUGH_MEMORY;
            goto cleanup;
        }

        stats->hit_offsets[0] = hit_offset;
        for (uint64_t i = 1; i < stats->nhits; i++)
            stats->hit_offsets[i] = eat64(&ptr);
    }

    *read_size = (size_t)(ptr - data);
    return H3C_OK;

cleanup:
    hmmd_stats_cleanup(stats);
    return rc;
}
