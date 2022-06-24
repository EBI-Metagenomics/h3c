#include "hmmd/stats.h"
#include "c_toolbelt/c_toolbelt.h"
#include "h3client/h3client.h"
#include "hmmd/utils.h"
#include "hmmd/zsetby.h"
#include "lite_pack/file/file.h"
#include "lite_pack/lite_pack.h"
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

enum h3c_rc hmmd_stats_deserialize(struct hmmd_stats *stats, size_t *read_size,
                                   unsigned char const *data)
{
    enum h3c_rc rc = H3C_OK;
    *read_size = 0;
    unsigned char const *ptr = data;

    stats->elapsed = eatf64(&ptr);
    stats->user = eatf64(&ptr);
    stats->sys = eatf64(&ptr);
    stats->Z = eatf64(&ptr);
    stats->domZ = eatf64(&ptr);

    size_t size = 0;
    if ((rc = hmmd_zsetby_unpack(&stats->Z_setby, &size, ptr))) goto cleanup;
    ptr += size;
    if ((rc = hmmd_zsetby_unpack(&stats->domZ_setby, &size, ptr))) goto cleanup;
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

enum h3c_rc hmmd_stats_pack(struct hmmd_stats const *stats, struct lip_file *f)
{
    lip_write_array_size(f, 17);
    lip_write_float(f, stats->elapsed);
    lip_write_float(f, stats->user);
    lip_write_float(f, stats->sys);

    lip_write_float(f, stats->Z);
    lip_write_float(f, stats->domZ);
    lip_write_int(f, stats->Z_setby);
    lip_write_int(f, stats->domZ_setby);

    lip_write_int(f, stats->nmodels);
    lip_write_int(f, stats->nseqs);
    lip_write_int(f, stats->n_past_msv);
    lip_write_int(f, stats->n_past_bias);
    lip_write_int(f, stats->n_past_vit);
    lip_write_int(f, stats->n_past_fwd);

    lip_write_int(f, stats->nhits);
    lip_write_int(f, stats->nreported);
    lip_write_int(f, stats->nincluded);

    lip_write_array_size(f, stats->nhits);
    for (uint64_t i = 0; i < stats->nhits; i++)
        lip_write_int(f, stats->hit_offsets[i]);

    return f->error ? H3C_FAILED_PACK : H3C_OK;
}
