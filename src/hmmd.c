#include "hmmd.h"
#include "c_toolbelt/c_toolbelt.h"
#include <stdlib.h>
#include <string.h>

void hmmd_status_unpack(struct hmmd_status *status, unsigned char const *data)
{
    uint32_t n32 = 0;
    memcpy(&n32, data, sizeof(n32));

    status->status = ctb_ntohl(n32);

    data += sizeof(n32);

    uint64_t n64 = 0;
    memcpy(&n64, data, sizeof(n64));
    status->msg_size = ctb_ntohll(n64);
}

static uint64_t eat64(unsigned char const **data)
{
    uint64_t n64 = 0;
    memcpy(&n64, *data, sizeof(uint64_t));
    *data += sizeof(uint64_t);
    return ctb_ntohll(n64);
}

static inline void to_double(double *dst, uint64_t h64)
{
    *dst = *((double *)&h64);
}

void hmmd_stats_init(struct hmmd_stats *stats)
{
    stats->elapsed = 0;
    memset(stats, 0, sizeof(*stats));
}

void hmmd_stats_cleanup(struct hmmd_stats *stats) { free(stats->hit_offsets); }

static enum h3c_rc hmmd_zsetby_unpack(enum hmmd_zsetby *dst,
                                      unsigned char const **data)
{
    enum h3c_rc rc = H3C_OK;
    switch (**data)
    {
    case 0:
        *dst = p7_ZSETBY_NTARGETS;
        break;
    case 1:
        *dst = p7_ZSETBY_OPTION;
        break;
    case 2:
        *dst = p7_ZSETBY_FILEINFO;
        break;
    default:
        rc = H3C_FAILED_UNPACK;
        break;
    }
    (*data)++;
    return rc;
}

enum h3c_rc hmmd_stats_unpack(struct hmmd_stats *stats,
                              unsigned char const *data)
{
    for (unsigned j = 0; j < 122; ++j) 
        printf("0x%hhx ", data[j]);
    printf("\n");
    uint64_t h64 = 0;
    enum h3c_rc rc = H3C_OK;

    to_double(&stats->elapsed, eat64(&data));
    to_double(&stats->user, eat64(&data));
    to_double(&stats->sys, eat64(&data));
    to_double(&stats->Z, eat64(&data));
    to_double(&stats->domZ, eat64(&data));

    if ((rc = hmmd_zsetby_unpack(&stats->Z_setby, &data))) goto cleanup;
    if ((rc = hmmd_zsetby_unpack(&stats->domZ_setby, &data))) goto cleanup;

    stats->nmodels = eat64(&data);
    stats->nseqs = eat64(&data);
    stats->n_past_msv = eat64(&data);
    stats->n_past_bias = eat64(&data);
    stats->n_past_vit = eat64(&data);
    stats->n_past_fwd = eat64(&data);
    stats->nhits = eat64(&data);
    stats->nreported = eat64(&data);
    stats->nincluded = eat64(&data);

    h64 = eat64(&data);
    if (h64 != UINT64_MAX)
    {
        stats->hit_offsets =
            ctb_realloc(stats->hit_offsets, stats->nhits * sizeof(uint64_t));
        if (!stats->hit_offsets)
        {
            rc = H3C_NOT_ENOUGH_MEMORY;
            goto cleanup;
        }

        stats->hit_offsets[0] = h64;
        for (uint64_t i = 1; i < stats->nhits; i++)
            stats->hit_offsets[i] = eat64(&data);
    }

cleanup:
    return rc;
}
