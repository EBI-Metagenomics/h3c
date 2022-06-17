#include "hmmd/tophits.h"
#include "h3client/rc.h"
#include "hmmd/hit.h"
#include <stdlib.h>
#include <string.h>

enum h3c_rc hmmd_tophits_init(struct hmmd_tophits *th)
{
    enum h3c_rc rc = H3C_OK;

    memset(th, 0, sizeof(*th));

    th->Nalloc = 16;

    if (!(th->hit = calloc(th->Nalloc, sizeof(*th->hit))))
    {
        rc = H3C_NOT_ENOUGH_MEMORY;
        goto cleanup;
    }

    if (!(th->unsrt = calloc(th->Nalloc, sizeof(*th->unsrt))))
    {
        rc = H3C_NOT_ENOUGH_MEMORY;
        goto cleanup;
    }

    th->hit[0] = th->unsrt;
    th->is_sorted_by_sortkey = true;

    return H3C_OK;

cleanup:
    if (th->hit) free(th->hit);
    if (th->unsrt) free(th->unsrt);
    return rc;
}

void hmmd_tophits_cleanup(struct hmmd_tophits *th)
{
    for (uint64_t i = 0; i < th->Nalloc; ++i)
        hmmd_hit_cleanup(th->unsrt + i);
    free(th->hit);
    free(th->unsrt);
    hmmd_tophits_init(th);
}
