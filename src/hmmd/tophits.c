#include "hmmd/tophits.h"
#include "ctb/ctb.h"
#include "h3client/rc.h"
#include "hmmd/domain.h"
#include "hmmd/hit.h"
#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <tgmath.h>

void hmmd_tophits_init(struct hmmd_tophits *th)
{
    memset(th, 0, sizeof(*th));
    th->hit = 0;
    th->unsrt = 0;
    th->is_sorted_by_sortkey = true;
}

static enum h3c_rc grow(struct hmmd_tophits *th, uint64_t nhits)
{
    enum h3c_rc rc = H3C_OK;

    if (!(th->hit = ctb_realloc(th->hit, sizeof(*th->hit) * nhits)))
    {
        rc = H3C_NOT_ENOUGH_MEMORY;
        goto cleanup;
    }

    size_t sz = nhits * sizeof(*th->unsrt);
    struct hmmd_hit *hits = realloc(th->unsrt, sz);
    if (!hits)
    {
        rc = H3C_NOT_ENOUGH_MEMORY;
        goto cleanup;
    }
    th->unsrt = hits;

    for (uint64_t i = th->nhits; i < nhits; ++i)
    {
        hmmd_hit_init(th->unsrt + i);
        ++th->nhits;
    }

    return H3C_OK;

cleanup:
    hmmd_tophits_cleanup(th);
    return rc;
}

static void shrink(struct hmmd_tophits *th, uint64_t nhits)
{
    for (uint64_t i = nhits; i < th->nhits; ++i)
    {
        th->hit[i] = 0;
        hmmd_hit_cleanup(th->unsrt + i);
    }

    th->nhits = nhits;
}

enum h3c_rc hmmd_tophits_setup(struct hmmd_tophits *th,
                               unsigned char const **ptr,
                               unsigned char const *end, uint64_t nhits,
                               uint64_t nreported, uint64_t nincluded)
{
    enum h3c_rc rc = H3C_OK;

    if (th->nhits < nhits)
        rc = grow(th, nhits);
    else
        shrink(th, nhits);

    if (rc) goto cleanup;

    th->nreported = nreported;
    th->nincluded = nincluded;
    th->is_sorted_by_seqidx = false;
    th->is_sorted_by_sortkey = true;

    for (uint64_t i = 0; i < nhits; ++i)
    {
        if ((rc = hmmd_hit_parse(th->unsrt + i, ptr, end))) goto cleanup;
        th->hit[i] = th->unsrt + i;
    }

    return H3C_OK;

cleanup:
    hmmd_tophits_cleanup(th);
    return rc;
}

void hmmd_tophits_cleanup(struct hmmd_tophits *th)
{
    for (uint64_t i = 0; i < th->nhits; ++i)
    {
        hmmd_hit_cleanup(th->unsrt + i);
    }
    free(th->hit);
    free(th->unsrt);
    hmmd_tophits_init(th);
}
