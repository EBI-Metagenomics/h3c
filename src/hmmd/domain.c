#include "hmmd/domain.h"
#include "c_toolbelt/c_toolbelt.h"
#include "h3client/rc.h"
#include "hmmd/alidisplay.h"
#include "hmmd/utils.h"
#include "lite_pack/file/file.h"
#include "lite_pack/lite_pack.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void hmmd_domain_init(struct hmmd_domain *dom)
{
    memset(dom, 0, offsetof(struct hmmd_domain, ad));
    hmmd_alidisplay_init(&dom->ad);
}

void hmmd_domain_cleanup(struct hmmd_domain *dom)
{
    if (dom->scores_per_pos) free(dom->scores_per_pos);
    hmmd_domain_init(dom);
}

enum h3c_rc hmmd_domain_unpack(struct hmmd_domain *dom, size_t *read_size,
                               unsigned char const *data)
{
    enum h3c_rc rc = H3C_OK;
    *read_size = 0;
    unsigned char const *ptr = data;

    size_t obj_size = eatu32(&ptr);
    dom->ienv = eatu64(&ptr);
    dom->jenv = eatu64(&ptr);
    dom->iali = eatu64(&ptr);
    dom->jali = eatu64(&ptr);
    dom->iorf = eatu64(&ptr);
    dom->jorf = eatu64(&ptr);
    dom->envsc = eatf32(&ptr);
    dom->domcorrection = eatf32(&ptr);
    dom->dombias = eatf32(&ptr);
    dom->oasc = eatf32(&ptr);
    dom->bitscore = eatf32(&ptr);
    dom->lnP = eatf64(&ptr);
    dom->is_reported = eatu32(&ptr);
    dom->is_included = eatu32(&ptr);

    printf("ienv: %lld\n", dom->ienv);
    printf("jenv: %lld\n", dom->jenv);
    printf("iali: %lld\n", dom->iali);
    printf("jali: %lld\n", dom->jali);
    printf("iorf: %lld\n", dom->iorf);
    printf("jorf: %lld\n", dom->jorf);

    printf("envsc: %f\n", dom->envsc);
    printf("domcorrection: %f\n", dom->domcorrection);
    printf("dombias: %f\n", dom->dombias);
    printf("oasc: %f\n", dom->oasc);
    printf("bitscore: %f\n", dom->bitscore);
    printf("lnP: %f\n", dom->lnP);

    printf("is_reported: %d\n", dom->is_reported);
    printf("is_included: %d\n", dom->is_included);

    dom->npos = eatu32(&ptr);

    if (dom->npos > 0)
    {
        size_t size = dom->npos * sizeof(float);
        dom->scores_per_pos = ctb_realloc(dom->scores_per_pos, size);

        for (unsigned i = 0; i < dom->npos; i++)
        {
            dom->scores_per_pos[i] = eatf32(&ptr);
        }
    }

    if (ptr != obj_size + data)
    {
        rc = H3C_INVALID_PACK;
        goto cleanup;
    }

    size_t size = 0;
    if ((rc = hmmd_alidisplay_unpack(&dom->ad, &size, ptr))) goto cleanup;
    ptr += size;

    *read_size = (size_t)(ptr - data);
    return H3C_OK;

cleanup:
    hmmd_domain_cleanup(dom);
    return rc;
}

enum h3c_rc hmmd_domain_pack(struct hmmd_domain const *dom, struct lip_file *f)
{
    lip_write_int(f, dom->ienv);
    lip_write_int(f, dom->jenv);

    lip_write_int(f, dom->iali);
    lip_write_int(f, dom->jali);

    lip_write_int(f, dom->iorf);
    lip_write_int(f, dom->jorf);

    lip_write_float(f, dom->envsc);
    lip_write_float(f, dom->domcorrection);
    lip_write_float(f, dom->dombias);
    lip_write_float(f, dom->oasc);
    lip_write_float(f, dom->bitscore);
    lip_write_float(f, dom->lnP);

    lip_write_bool(f, dom->is_reported);
    lip_write_bool(f, dom->is_included);

    lip_write_array_size(f, dom->npos);
    for (size_t i = 0; i < dom->npos; i++)
        lip_write_float(f, dom->scores_per_pos[i]);

    hmmd_alidisplay_pack(&dom->ad, f);

    return f->error ? H3C_FAILED_PACK : H3C_OK;
}
