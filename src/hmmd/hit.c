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

enum h3c_rc hmmd_hit_parse(struct hmmd_hit *hit, size_t *read_size,
                           unsigned char const *data)
{
    enum h3c_rc rc = H3C_OK;
    *read_size = 0;
    unsigned char const *ptr = data;

    uint32_t obj_size = eatu32(&ptr);

    // Member window_length apparently being set with random values.
    // Skipping it for now.
    eati32(&ptr);
    // hit->window_length = eati32(&ptr);

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
    uint32_t ndom = eatu32(&ptr);

    hit->flags = eatu32(&ptr);
    hit->nreported = eatu32(&ptr);
    hit->nincluded = eatu32(&ptr);
    hit->best_domain = eatu32(&ptr);
    hit->seqidx = eatu64(&ptr);
    // Member window_length apparently being set with random values.
    // Skipping it for now.
    eatu64(&ptr);
    // hit->subseq_start = eatu64(&ptr);

    uint8_t presence = eatu8(&ptr);

    if ((rc = eatstr(&hit->name, &ptr))) goto cleanup;

    if (presence & ACC_PRESENT)
    {
        if ((rc = eatstr(&hit->acc, &ptr))) goto cleanup;
    }

    if (presence & DESC_PRESENT)
    {
        if ((rc = eatstr(&hit->desc, &ptr))) goto cleanup;
    }

    if (ptr != obj_size + data)
    {
        rc = H3C_FAILED_PARSE;
        goto cleanup;
    }

    if (ndom > hit->ndom)
    {
        struct hmmd_domain *dcl = realloc(hit->dcl, ndom * sizeof(*hit->dcl));
        if (!dcl)
        {
            rc = H3C_NOT_ENOUGH_MEMORY;
            goto cleanup;
        }

        hit->dcl = dcl;
        for (uint32_t i = hit->ndom; i < ndom; i++)
        {
            hmmd_domain_init(hit->dcl + i);
            ++hit->ndom;
        }
    }
    else
    {
        for (uint32_t i = ndom; i < hit->ndom; i++)
            hmmd_domain_cleanup(hit->dcl + i);
        hit->ndom = ndom;
    }

    for (unsigned i = 0; i < hit->ndom; i++)
    {
        size_t size = 0;
        if ((rc = hmmd_domain_parse(&(hit->dcl[i]), &size, ptr))) goto cleanup;
        ptr += size;
    }

    *read_size = (size_t)(ptr - data);
    return H3C_OK;

cleanup:
    hmmd_hit_cleanup(hit);
    return rc;
}
