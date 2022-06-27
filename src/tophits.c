#include "tophits.h"
#include "del.h"
#include "h3client/rc.h"
#include "hit.h"
#include "lite_pack/lite_pack.h"
#include "tophits.h"
#include <stdlib.h>
#include <string.h>

void tophits_init(struct h3c_tophits *th) { memset(th, 0, sizeof(*th)); }

static enum h3c_rc grow(struct h3c_tophits *th, uint64_t nhits)
{
    enum h3c_rc rc = H3C_OK;

    size_t sz = nhits * sizeof(*th->hits);
    struct h3c_hit *hits = realloc(th->hits, sz);
    if (!hits)
    {
        rc = H3C_NOT_ENOUGH_MEMORY;
        goto cleanup;
    }
    th->hits = hits;

    for (uint64_t i = th->nhits; i < nhits; ++i)
    {
        if ((rc = hit_init(th->hits + i))) goto cleanup;
        ++th->nhits;
    }

    return H3C_OK;

cleanup:
    tophits_cleanup(th);
    return rc;
}

static void shrink(struct h3c_tophits *th, uint64_t nhits)
{
    for (uint64_t i = nhits; i < th->nhits; ++i)
        hit_cleanup(th->hits + i);

    th->nhits = nhits;
}

enum h3c_rc tophits_setup(struct h3c_tophits *th, uint64_t nhits)
{
    if (th->nhits < nhits) return grow(th, nhits);
    shrink(th, nhits);
    return H3C_OK;
}

void tophits_cleanup(struct h3c_tophits *th)
{
    for (uint64_t i = 0; i < th->nhits; ++i)
        hit_cleanup(th->hits + i);
    DEL(th->hits);
    th->nhits = 0;
}

enum h3c_rc tophits_pack(struct h3c_tophits const *th, struct lip_file *f)
{
    lip_write_array_size(f, 5);

    lip_write_map_size(f, 1);
    lip_write_cstr(f, "hits");
    lip_write_array_size(f, th->nhits);
    if (lip_file_error(f)) return H3C_FAILED_PACK;

    for (uint64_t i = 0; i < th->nhits; ++i)
    {
        enum h3c_rc rc = hit_pack(th->hits + i, f);
        if (rc) return rc;
    }

    lip_write_int(f, th->nreported);
    lip_write_int(f, th->nincluded);
    lip_write_bool(f, th->is_sorted_by_sortkey);
    lip_write_bool(f, th->is_sorted_by_seqidx);

    return lip_file_error(f) ? H3C_FAILED_PACK : H3C_OK;
}
