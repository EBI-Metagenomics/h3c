#include "domain.h"
#include "alidisplay.h"
#include "c_toolbelt/c_toolbelt.h"
#include "h3client/rc.h"
#include "lite_pack/lite_pack.h"
#include "utils.h"
#include <stdlib.h>
#include <string.h>

void domain_init(struct domain *dom)
{
    memset(dom, 0, offsetof(struct domain, ad));
    alidisplay_init(&dom->ad);
}

static enum h3c_rc grow_scores(struct domain *dom, unsigned size)
{
    size_t sz = size * sizeof(*dom->pos_score);
    if (!(dom->pos_score = ctb_realloc(dom->pos_score, sz)))
    {
        domain_cleanup(dom);
        return H3C_NOT_ENOUGH_MEMORY;
    }
    return H3C_OK;
}

static void shrink_scores(struct domain *dom, unsigned size)
{
    dom->pos_score_size = size;
}

enum h3c_rc domain_setup(struct domain *dom, unsigned scores_size)
{
    if (dom->pos_score_size < scores_size) return grow_scores(dom, scores_size);
    shrink_scores(dom, scores_size);
    return H3C_OK;
}

void domain_cleanup(struct domain *dom)
{
    DEL(dom->pos_score);
    dom->pos_score_size = 0;
    alidisplay_cleanup(&dom->ad);
}

enum h3c_rc domain_pack(struct domain const *dom, struct lip_file *f)
{
    lip_write_array_size(f, 14);

    lip_write_int(f, dom->ienv);
    lip_write_int(f, dom->jenv);

    lip_write_int(f, dom->iali);
    lip_write_int(f, dom->jali);

    lip_write_float(f, dom->envsc);
    lip_write_float(f, dom->domcorrection);
    lip_write_float(f, dom->dombias);
    lip_write_float(f, dom->oasc);
    lip_write_float(f, dom->bitscore);
    lip_write_float(f, dom->lnP);

    lip_write_bool(f, dom->is_reported);
    lip_write_bool(f, dom->is_included);

    lip_write_array_size(f, dom->pos_score_size);
    for (unsigned long i = 0; i < dom->pos_score_size; i++)
        lip_write_float(f, dom->pos_score[i]);

    lip_write_map_size(f, 1);
    lip_write_cstr(f, "alidisplay");

    return alidisplay_pack(&dom->ad, f);
}

enum h3c_rc domain_unpack(struct domain *dom, struct lip_file *f)
{
    enum h3c_rc rc = H3C_FAILED_UNPACK;

    if (!expect_array_size(f, 14)) goto cleanup;

    lip_read_int(f, &dom->ienv);
    lip_read_int(f, &dom->jenv);

    lip_read_int(f, &dom->iali);
    lip_read_int(f, &dom->jali);

    lip_read_float(f, &dom->envsc);
    lip_read_float(f, &dom->domcorrection);
    lip_read_float(f, &dom->dombias);
    lip_read_float(f, &dom->oasc);
    lip_read_float(f, &dom->bitscore);
    lip_read_float(f, &dom->lnP);

    lip_read_bool(f, &dom->is_reported);
    lip_read_bool(f, &dom->is_included);

    unsigned size = 0;
    lip_read_array_size(f, &size);
    if ((rc = domain_setup(dom, size))) goto cleanup;

    for (unsigned long i = 0; i < dom->pos_score_size; i++)
        lip_read_float(f, dom->pos_score + i);

    if (!expect_map_size(f, 1)) goto cleanup;
    if (!expect_key(f, "alidisplay")) goto cleanup;

    if ((rc = alidisplay_unpack(&dom->ad, f))) goto cleanup;

    return H3C_OK;

cleanup:
    domain_cleanup(dom);
    return rc;
}
