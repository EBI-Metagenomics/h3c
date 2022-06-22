#include "hmmd/tophits.h"
#include "c_toolbelt/c_toolbelt.h"
#include "h3client/rc.h"
#include "hmmd/hit.h"
#include <stdlib.h>
#include <string.h>

enum h3c_rc hmmd_tophits_init(struct hmmd_tophits *th)
{
    enum h3c_rc rc = H3C_OK;

    memset(th, 0, sizeof(*th));

    th->Nalloc = 0;
    th->hit = 0;
    th->unsrt= 0;
    th->is_sorted_by_sortkey = true;

    return H3C_OK;
}

enum h3c_rc hmmd_tophits_setup(struct hmmd_tophits *th,
                               unsigned char const *data, uint64_t nhits,
                               uint64_t nreported, uint64_t nincluded)
{
    enum h3c_rc rc = H3C_OK;

    th->hit = 0;
    th->unsrt = 0;

    if (!(th->hit = ctb_realloc(th->hit, sizeof(*th->hit) * nhits)))
    {
        rc = H3C_NOT_ENOUGH_MEMORY;
        goto cleanup;
    }

    if (!(th->unsrt = ctb_realloc(th->unsrt, sizeof(*th->unsrt) * nhits)))
    {
        rc = H3C_NOT_ENOUGH_MEMORY;
        goto cleanup;
    }

    th->N = nhits;
    th->nreported = nreported;
    th->nincluded = nincluded;
    th->is_sorted_by_seqidx = false;
    th->is_sorted_by_sortkey = true;

    unsigned char const *ptr = data;
    for (uint64_t i = 0; i < nhits; ++i)
    {
        hmmd_hit_init(th->unsrt + i);
        size_t size = 0;
        if ((rc = hmmd_hit_unpack(th->unsrt + i, &size, ptr))) goto cleanup;
        ptr += size;
        th->hit[i] = th->unsrt + i;
    }

cleanup:
    hmmd_tophits_cleanup(th);
    return rc;
}

void hmmd_tophits_cleanup(struct hmmd_tophits *th)
{
    for (uint64_t i = 0; i < th->N; ++i)
    {
        hmmd_hit_cleanup(th->unsrt + i);
    }
    free(th->hit);
    free(th->unsrt);
    hmmd_tophits_init(th);
}
