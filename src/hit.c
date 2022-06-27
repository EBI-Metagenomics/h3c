#include "hit.h"
#include "del.h"
#include "domain.h"
#include "h3client/rc.h"
#include "lite_pack/lite_pack.h"
#include <stdlib.h>
#include <string.h>

enum h3c_rc hit_init(struct h3c_hit *hit)
{
    memset(hit, 0, sizeof(*hit));

    if (!(hit->name = malloc(1))) goto cleanup;
    if (!(hit->acc = malloc(1))) goto cleanup;
    if (!(hit->desc = malloc(1))) goto cleanup;

    return H3C_OK;

cleanup:
    hit_cleanup(hit);
    return H3C_NOT_ENOUGH_MEMORY;
}

static enum h3c_rc grow(struct h3c_hit *hit, uint32_t ndomains)
{
    size_t sz = ndomains * sizeof(*hit->domains);
    struct h3c_domain *domains = realloc(hit->domains, sz);
    if (!domains) goto cleanup;
    hit->domains = domains;

    for (uint32_t i = hit->ndomains; i < ndomains; ++i)
    {
        domain_init(hit->domains + i);
        ++hit->ndomains;
    }

    return H3C_OK;

cleanup:
    hit_cleanup(hit);
    return H3C_NOT_ENOUGH_MEMORY;
}

static void shrink(struct h3c_hit *hit, uint32_t ndomains)
{
    for (uint32_t i = ndomains; i < hit->ndomains; ++i)
        domain_cleanup(hit->domains + i);

    hit->ndomains = ndomains;
}

enum h3c_rc hit_setup(struct h3c_hit *hit, uint32_t ndomains)
{
    if (hit->ndomains < ndomains) return grow(hit, ndomains);
    shrink(hit, ndomains);
    return H3C_OK;
}

void hit_cleanup(struct h3c_hit *hit)
{
    DEL(hit->name);
    DEL(hit->acc);
    DEL(hit->desc);

    for (uint32_t i = 0; i < hit->ndomains; ++i)
        domain_cleanup(hit->domains + i);

    hit->ndomains = 0;
    DEL(hit->domains);
}

enum h3c_rc hit_pack(struct h3c_hit const *hit, struct lip_file *f)
{
    lip_write_array_size(f, 20);

    lip_write_cstr(f, hit->name);
    lip_write_cstr(f, hit->acc);
    lip_write_cstr(f, hit->desc);
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

    lip_write_int(f, hit->flags);
    lip_write_int(f, hit->nreported);
    lip_write_int(f, hit->nincluded);
    lip_write_int(f, hit->best_domain);

    lip_write_map_size(f, 1);
    lip_write_cstr(f, "domains");
    lip_write_array_size(f, hit->ndomains);
    if (lip_file_error(f)) return H3C_FAILED_PACK;

    for (unsigned i = 0; i < hit->ndomains; ++i)
    {
        enum h3c_rc rc = domain_pack(hit->domains + i, f);
        if (rc) return rc;
    }

    return H3C_OK;
}
