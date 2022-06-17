#include "hmmd/hit.h"
#include "c_toolbelt/c_toolbelt.h"
#include "h3client/h3client.h"
#include "hmmd/domain.h"
#include "hmmd/utils.h"
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

    uint8_t presence = eat8(&ptr);

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
