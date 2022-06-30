#include "hmmd/hit.h"
#include "c_toolbelt/c_toolbelt.h"
#include "h3client/h3client.h"
#include "hmmd/domain.h"
#include "utils.h"
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

static enum h3c_rc parse_strings(struct hmmd_hit *hit, uint8_t presence,
                                 size_t size, unsigned char const **ptr)
{
    unsigned n = 1 + !!(presence & ACC_PRESENT) + !!(presence & DESC_PRESENT);
    if (!expect_n_strings(size, (char const *)*ptr, n)) return H3C_FAILED_PARSE;

    enum h3c_rc rc = H3C_OK;

    if ((rc = eatstr(&hit->name, ptr))) return rc;

    if (presence & ACC_PRESENT)
    {
        if ((rc = eatstr(&hit->acc, ptr))) return rc;
    }

    if (presence & DESC_PRESENT)
    {
        if ((rc = eatstr(&hit->desc, ptr))) return rc;
    }

    return H3C_OK;
}

enum h3c_rc hmmd_hit_parse(struct hmmd_hit *hit, unsigned char const **ptr,
                           unsigned char const *end)
{
    enum h3c_rc rc = H3C_OK;

    ESCAPE_OVERRUN(rc, *ptr, end, 2 * sizeof(uint32_t));

    // Skips object size
    (void)eatu32(ptr);

    // Skips window_length for now
    (void)eati32(ptr);

    ESCAPE_OVERRUN(rc, *ptr, end, 4 * sizeof(double) + 3 * sizeof(float));
    hit->sortkey = eatf64(ptr);
    hit->score = eatf32(ptr);
    hit->pre_score = eatf32(ptr);
    hit->sum_score = eatf32(ptr);

    hit->lnP = eatf64(ptr);
    hit->pre_lnP = eatf64(ptr);
    hit->sum_lnP = eatf64(ptr);

    ESCAPE_OVERRUN(rc, *ptr, end, 5 * sizeof(uint32_t) + sizeof(float));
    hit->nexpected = eatf32(ptr);
    hit->nregions = eatu32(ptr);
    hit->nclustered = eatu32(ptr);
    hit->noverlaps = eatu32(ptr);
    hit->nenvelopes = eatu32(ptr);
    uint32_t ndom = eatu32(ptr);

    ESCAPE_OVERRUN(rc, *ptr, end, 4 * sizeof(uint32_t) + 2 * sizeof(uint64_t));
    hit->flags = eatu32(ptr);
    hit->nreported = eatu32(ptr);
    hit->nincluded = eatu32(ptr);
    hit->best_domain = eatu32(ptr);
    hit->seqidx = eatu64(ptr);
    // Skips subseq_start for now
    (void)eatu64(ptr);

    ESCAPE_OVERRUN(rc, *ptr, end, sizeof(uint8_t));
    uint8_t presence = eatu8(ptr);

    if ((rc = parse_strings(hit, presence, (end - *ptr), ptr))) goto cleanup;

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

    for (uint32_t i = 0; i < hit->ndom; i++)
    {
        if ((rc = hmmd_domain_parse(hit->dcl + i, ptr, end))) goto cleanup;
    }

    return H3C_OK;

cleanup:
    hmmd_hit_cleanup(hit);
    return rc;
}
