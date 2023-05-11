#include "hmmd/hit.h"
#include "h3c/h3c.h"
#include "hmmd/domain.h"
#include "utils.h"
#include <stdlib.h>
#include <string.h>

void h3c_hmmd_hit_init(struct hmmd_hit *hit) { memset(hit, 0, sizeof(*hit)); }

void h3c_hmmd_hit_cleanup(struct hmmd_hit *hit)
{
    if (hit->name) free(hit->name);
    if (hit->acc) free(hit->acc);
    if (hit->desc) free(hit->desc);

    for (unsigned i = 0; i < hit->ndom; i++)
        h3c_hmmd_domain_cleanup(hit->dcl + i);

    free(hit->dcl);
    h3c_hmmd_hit_init(hit);
}

#define ACC_PRESENT (1 << 0)
#define DESC_PRESENT (1 << 1)

static int parse_strings(struct hmmd_hit *hit, uint8_t presence, size_t size,
                         unsigned char const **ptr)
{
    unsigned n = 1 + !!(presence & ACC_PRESENT) + !!(presence & DESC_PRESENT);
    if (!h3c_expect_n_strings(size, (char const *)*ptr, n)) return H3C_EPARSE;

    int rc = 0;

    if ((rc = h3c_eatstr(&hit->name, ptr))) return rc;

    if (presence & ACC_PRESENT)
    {
        if ((rc = h3c_eatstr(&hit->acc, ptr))) return rc;
    }

    if (presence & DESC_PRESENT)
    {
        if ((rc = h3c_eatstr(&hit->desc, ptr))) return rc;
    }

    return 0;
}

int h3c_hmmd_hit_parse(struct hmmd_hit *hit, unsigned char const **ptr,
                       unsigned char const *end)
{
    int rc = 0;

    ESCAPE_OVERRUN(rc, *ptr, end, 2 * sizeof(uint32_t));

    // Skips object size
    (void)h3c_eatu32(ptr);

    // Skips window_length for now
    (void)h3c_eati32(ptr);

    ESCAPE_OVERRUN(rc, *ptr, end, 4 * sizeof(double) + 3 * sizeof(float));
    hit->sortkey = h3c_eatf64(ptr);
    hit->score = h3c_eatf32(ptr);
    hit->pre_score = h3c_eatf32(ptr);
    hit->sum_score = h3c_eatf32(ptr);

    hit->lnP = h3c_eatf64(ptr);
    hit->pre_lnP = h3c_eatf64(ptr);
    hit->sum_lnP = h3c_eatf64(ptr);

    ESCAPE_OVERRUN(rc, *ptr, end, 5 * sizeof(uint32_t) + sizeof(float));
    hit->nexpected = h3c_eatf32(ptr);
    hit->nregions = h3c_eatu32(ptr);
    hit->nclustered = h3c_eatu32(ptr);
    hit->noverlaps = h3c_eatu32(ptr);
    hit->nenvelopes = h3c_eatu32(ptr);
    uint32_t ndom = h3c_eatu32(ptr);

    ESCAPE_OVERRUN(rc, *ptr, end, 4 * sizeof(uint32_t) + 2 * sizeof(uint64_t));
    hit->flags = h3c_eatu32(ptr);
    hit->nreported = h3c_eatu32(ptr);
    hit->nincluded = h3c_eatu32(ptr);
    hit->best_domain = h3c_eatu32(ptr);
    // Skips seqidx
    (void)h3c_eatu64(ptr);
    // Skips subseq_start for now
    (void)h3c_eatu64(ptr);

    ESCAPE_OVERRUN(rc, *ptr, end, sizeof(uint8_t));
    uint8_t presence = h3c_eatu8(ptr);

    if ((rc = parse_strings(hit, presence, (end - *ptr), ptr))) goto cleanup;

    if (ndom > hit->ndom)
    {
        struct hmmd_domain *dcl = realloc(hit->dcl, ndom * sizeof(*hit->dcl));
        if (!dcl)
        {
            rc = H3C_ENOMEM;
            goto cleanup;
        }

        hit->dcl = dcl;
        for (uint32_t i = hit->ndom; i < ndom; i++)
        {
            h3c_hmmd_domain_init(hit->dcl + i);
            ++hit->ndom;
        }
    }
    else
    {
        for (uint32_t i = ndom; i < hit->ndom; i++)
            h3c_hmmd_domain_cleanup(hit->dcl + i);
        hit->ndom = ndom;
    }

    for (uint32_t i = 0; i < hit->ndom; i++)
    {
        if ((rc = h3c_hmmd_domain_parse(hit->dcl + i, ptr, end))) goto cleanup;
    }

    return 0;

cleanup:
    h3c_hmmd_hit_cleanup(hit);
    return rc;
}
