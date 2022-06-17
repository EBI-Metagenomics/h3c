#include "hmmd/tophits.h"
#include <stdlib.h>

struct hmmd_tophits *hmmd_tophits_new(void)
{
    struct hmmd_tophits *th = malloc(sizeof(*th));
    if (!th) return 0;

    th->hit = 0;
    th->unsrt = 0;

    th->Nalloc = 16;
    if (!(th->hit = malloc(sizeof(*th->hit) * th->Nalloc))) goto cleanup;
    if (!(th->unsrt = malloc(sizeof(*th->unsrt) * th->Nalloc))) goto cleanup;
    th->hit[0] = th->unsrt;
    th->N = 0;
    th->nreported = 0;
    th->nincluded = 0;
    th->is_sorted_by_sortkey = 1;
    th->is_sorted_by_seqidx = 0;

    return th;

cleanup:
    if (th->hit) free(th->hit);
    if (th->unsrt) free(th->unsrt);
    return 0;
}

void hmmd_tophits_del(struct hmmd_tophits const *th)
{
    free(th->hit);
    free(th->unsrt);
    free((void *)th);
}
