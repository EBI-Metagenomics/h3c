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

enum h3c_rc hmmd_stats_parse(struct hmmd_stats *stats, size_t *read_size,
                                   unsigned char const *data)
{
    enum h3c_rc rc = H3C_OK;
    *read_size = 0;
    unsigned char const *ptr = data;

    (void)eatf64(&ptr);
    (void)eatf64(&ptr);
    (void)eatf64(&ptr);
    // stats->elapsed = eatf64(&ptr);
    // stats->user = eatf64(&ptr);
    // stats->sys = eatf64(&ptr);

    stats->Z = eatf64(&ptr);
    stats->domZ = eatf64(&ptr);

    size_t size = 0;
    if ((rc = hmmd_zsetby_parse(&stats->Z_setby, &size, ptr))) goto cleanup;
    ptr += size;
    if ((rc = hmmd_zsetby_parse(&stats->domZ_setby, &size, ptr))) goto cleanup;
    ptr += size;

    stats->nmodels = eatu64(&ptr);
    stats->nseqs = eatu64(&ptr);
    stats->n_past_msv = eatu64(&ptr);
    stats->n_past_bias = eatu64(&ptr);
    stats->n_past_vit = eatu64(&ptr);
    stats->n_past_fwd = eatu64(&ptr);
    stats->nhits = eatu64(&ptr);
    stats->nreported = eatu64(&ptr);
    stats->nincluded = eatu64(&ptr);

    uint64_t hit_offset = eatu64(&ptr);
    assert(!(hit_offset == UINT64_MAX && stats->nhits > 0));
    if (hit_offset != UINT64_MAX)
    {
        assert(stats->nhits > 0);
        size = stats->nhits * sizeof(uint64_t);
        stats->hit_offsets = ctb_realloc(stats->hit_offsets, size);
        if (!stats->hit_offsets)
        {
            rc = H3C_NOT_ENOUGH_MEMORY;
            goto cleanup;
        }

        stats->hit_offsets[0] = hit_offset;
        for (uint64_t i = 1; i < stats->nhits; i++)
            stats->hit_offsets[i] = eatu64(&ptr);
    }

    *read_size = (size_t)(ptr - data);
    return H3C_OK;

cleanup:
    hmmd_stats_cleanup(stats);
    return rc;
}
