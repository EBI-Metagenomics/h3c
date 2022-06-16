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

static uint32_t eat32(unsigned char const **data)
{
    uint32_t n32 = 0;
    memcpy(&n32, *data, sizeof(uint32_t));
    *data += sizeof(uint32_t);
    return ctb_ntohl(n32);
}

static uint8_t eat8(unsigned char const **data)
{
    uint8_t n8 = 0;
    memcpy(&n8, *data, sizeof(uint8_t));
    *data += sizeof(uint8_t);
    return n8;
}

static enum h3c_rc eatstr(char **dst, unsigned char const **data)
{
    size_t size = strlen((char const *)*data) + 1;
    if (!(*dst = ctb_realloc(*dst, size))) return H3C_NOT_ENOUGH_MEMORY;
    memcpy(*dst, *data, size);
    *data += size;
    return H3C_OK;
}

static inline void to_double(double *dst, uint64_t h64)
{
    *dst = *((double *)&h64);
}

static inline void to_float(float *dst, uint32_t h32)
{
    *dst = *((float *)&h32);
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

enum h3c_rc hmmd_stats_unpack(struct hmmd_stats *stats, size_t *read_size,
                              unsigned char const *data)
{
    *read_size = 0;
    unsigned char const *ptr = data;
    enum h3c_rc rc = H3C_OK;

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

#define ACC_PRESENT (1 << 0)
#define DESC_PRESENT (1 << 1)

void hmmd_hit_cleanup(struct hmmd_hit *hit)
{
    free(hit->name);
    free(hit->acc);
    free(hit->desc);
    free(hit->dcl);
}

enum h3c_rc hmmd_hit_unpack(struct hmmd_hit *hit, size_t *read_size,
                            unsigned char const *data)
{
    enum h3c_rc rc = H3C_OK;
    *read_size = 0;
    unsigned char const *ptr = data;
    // How much space does the variable-length portion of the
    // serialized object take up?
    uint32_t obj_size = eat32(&ptr);

    hit->window_length = eat32(&ptr);
    to_double(&hit->sortkey, eat64(&ptr));
    to_float(&hit->score, eat32(&ptr));
    to_float(&hit->pre_score, eat32(&ptr));
    to_float(&hit->sum_score, eat32(&ptr));
    to_double(&hit->lnP, eat64(&ptr));
    to_double(&hit->pre_lnP, eat64(&ptr));
    to_double(&hit->sum_lnP, eat64(&ptr));
    to_float(&hit->nexpected, eat32(&ptr));
    hit->nregions = eat32(&ptr);
    hit->nclustered = eat32(&ptr);
    hit->noverlaps = eat32(&ptr);
    hit->nenvelopes = eat32(&ptr);
    hit->ndom = eat32(&ptr);
    hit->flags = eat32(&ptr);
    hit->nreported = eat32(&ptr);
    hit->nincluded = eat32(&ptr);
    hit->best_domain = eat32(&ptr);
    hit->seqidx = eat64(&ptr);
    hit->subseq_start = eat64(&ptr);

    uint8_t presence_flags = eat8(&ptr);

    if ((rc = eatstr(&hit->name, &ptr))) goto cleanup;

    if (presence_flags & ACC_PRESENT)
    {
        if ((rc = eatstr(&hit->acc, &ptr))) goto cleanup;
    }
    else
    {
        free(hit->acc);
        hit->acc = 0;
    }

    if (presence_flags & DESC_PRESENT)
    {
        if ((rc = eatstr(&hit->desc, &ptr))) goto cleanup;
    }
    else
    {
        free(hit->desc);
        hit->desc = 0;
    }

    // sanity check
    if ((size_t)(ptr - data) != obj_size)
    {
        printf("Size of serialized object did not match");
        printf("expected in p7_hit_Deserialize");
    }

    hit->dcl = ctb_realloc(hit->dcl, hit->ndom * sizeof(*hit->dcl));

    // reset n to point just past fixed-length fields
    // *n = ptr - buf;

    for (int i = 0; i < hit->ndom; i++)
    {
        // set internal pointers to known values so that
        // domain_Deserialize does the right thing
        hit->dcl[i].scores_per_pos = NULL;
        hit->dcl[i].ad = NULL;
        // int ret_code = p7_domain_Deserialize(buf, n, &(hit->dcl[i]));
        // if (ret_code != eslOK)
        // {
        //     return ret_code;
        // }
    }

    return H3C_OK;

cleanup:
    return rc;
}
