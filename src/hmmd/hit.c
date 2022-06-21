#include "hmmd/hit.h"
#include "c_toolbelt/c_toolbelt.h"
#include "h3client/h3client.h"
#include "hmmd/domain.h"
#include "hmmd/utils.h"
#include "lite_pack/file/file.h"
#include "lite_pack/lite_pack.h"
#include <stdlib.h>
#include <string.h>

void hmmd_hit_init(struct hmmd_hit *hit) { memset(hit, 0, sizeof(*hit)); }

void hmmd_hit_cleanup(struct hmmd_hit *hit)
{
    if (hit->name) free(hit->name);
    if (hit->acc) free(hit->acc);
    if (hit->desc) free(hit->desc);
    for (unsigned i = 0; i < hit->ndom; i++)
        hmmd_domain_cleanup(hit->dcl + i);
    free(hit->dcl);
    hmmd_hit_init(hit);
}

#define ACC_PRESENT (1 << 0)
#define DESC_PRESENT (1 << 1)

enum h3c_rc hmmd_hit_unpack(struct hmmd_hit *hit, size_t *read_size,
                            unsigned char const *data)
{
    enum h3c_rc rc = H3C_OK;
    *read_size = 0;
    unsigned char const *ptr = data;

    uint32_t obj_size = eatu32(&ptr);
    hit->window_length = eatu32(&ptr);
    hit->sortkey = eatf64(&ptr);
    hit->score = eatf32(&ptr);
    hit->pre_score = eatf32(&ptr);
    hit->sum_score = eatf32(&ptr);
    hit->lnP = eatf64(&ptr);
    hit->pre_lnP = eatf64(&ptr);
    hit->sum_lnP = eatf64(&ptr);
    hit->nexpected = eatf32(&ptr);
    hit->nregions = eatu32(&ptr);
    hit->nclustered = eatu32(&ptr);
    hit->noverlaps = eatu32(&ptr);
    hit->nenvelopes = eatu32(&ptr);
    hit->ndom = eatu32(&ptr);
    hit->flags = eatu32(&ptr);
    hit->nreported = eatu32(&ptr);
    hit->nincluded = eatu32(&ptr);
    hit->best_domain = eatu32(&ptr);
    hit->seqidx = eatu64(&ptr);
    hit->subseq_start = eatu64(&ptr);

    uint8_t presence = eatu8(&ptr);

    if ((rc = eatstr(&hit->name, &ptr))) goto cleanup;

    if (presence & ACC_PRESENT)
    {
        if ((rc = eatstr(&hit->acc, &ptr))) goto cleanup;
    }
    else
    {
        free(hit->acc);
        hit->acc = 0;
    }

    if (presence & DESC_PRESENT)
    {
        if ((rc = eatstr(&hit->desc, &ptr))) goto cleanup;
    }
    else
    {
        free(hit->desc);
        hit->desc = 0;
    }

    if (ptr != obj_size + data)
    {
        rc = H3C_INVALID_PACK;
        goto cleanup;
    }

    hit->dcl = ctb_realloc(hit->dcl, hit->ndom * sizeof(*hit->dcl));

    printf("name: %s\n", hit->name);
    printf("acc: %s\n", hit->acc);
    printf("desc: %s\n", hit->desc);
    printf("window_length: %d\n", hit->window_length);
    printf("sortkey: %f\n", hit->sortkey);
    printf("score: %f\n", hit->score);
    printf("pre_score: %f\n", hit->pre_score);
    printf("sum_score: %f\n", hit->sum_score);
    printf("lnP: %f\n", hit->lnP);
    printf("pre_lnP: %f\n", hit->pre_lnP);
    printf("sum_lnP: %f\n", hit->sum_lnP);

    printf("nexpected: %f\n", hit->nexpected);
    printf("nregions: %d\n", hit->nregions);
    printf("nclustered: %d\n", hit->nclustered);
    printf("noverlaps: %d\n", hit->noverlaps);
    printf("nenvelopes: %d\n", hit->nenvelopes);
    printf("ndom: %d\n", hit->ndom);

    printf("flags: %d\n", hit->flags);
    printf("nreported: %d\n", hit->nreported);
    printf("nincluded: %d\n", hit->nincluded);
    printf("best_domain: %d\n", hit->best_domain);

    printf("seqidx: %lld\n", hit->seqidx);
    printf("subseq_start: %lld\n", hit->subseq_start);

    for (unsigned i = 0; i < hit->ndom; i++)
    {
        hmmd_domain_init(hit->dcl + i);
        size_t size = 0;
        if ((rc = hmmd_domain_unpack(&(hit->dcl[i]), &size, ptr))) goto cleanup;
        ptr += size;
    }

    *read_size = (size_t)(ptr - data);
    return H3C_OK;

cleanup:
    hmmd_hit_cleanup(hit);
    return rc;
}

enum h3c_rc hmmd_hit_pack(struct hmmd_hit const *hit, struct lip_file *f)
{
    lip_write_cstr(f, hit->name);
    lip_write_cstr(f, hit->acc);
    lip_write_cstr(f, hit->desc);
    lip_write_int(f, hit->window_length);
    lip_write_float(f, hit->sortkey);

    lip_write_float(f, hit->score);
    lip_write_float(f, hit->pre_score);
    lip_write_float(f, hit->sum_score);

    lip_write_float(f, hit->lnP);
    lip_write_float(f, hit->pre_lnP);
    lip_write_float(f, hit->sum_lnP);

    lip_write_float(f, hit->nexpected);
    lip_write_int(f, hit->nregions);
    lip_write_int(f, hit->nclustered);
    lip_write_int(f, hit->noverlaps);
    lip_write_int(f, hit->nenvelopes);
    lip_write_int(f, hit->ndom);

    lip_write_int(f, hit->flags);
    lip_write_int(f, hit->nreported);
    lip_write_int(f, hit->nincluded);
    lip_write_int(f, hit->best_domain);

    lip_write_int(f, hit->seqidx);
    lip_write_int(f, hit->subseq_start);

    for (unsigned i = 0; i < hit->ndom; ++i)
        hmmd_domain_pack(hit->dcl + i, f);

    return f->error ? H3C_FAILED_PACK : H3C_OK;
}