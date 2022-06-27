#include "hmmd/domain.h"
#include "c_toolbelt/c_toolbelt.h"
#include "h3client/rc.h"
#include "hmmd/alidisplay.h"
#include "hmmd/utils.h"
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
    hmmd_alidisplay_cleanup(&dom->ad);
    hmmd_domain_init(dom);
}

enum h3c_rc hmmd_domain_parse(struct hmmd_domain *dom, size_t *read_size,
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

    // Members iorf and jorf apparently being set with random values.
    // Skipping them for now.
    eati64(&ptr);
    eati64(&ptr);

    // dom->iorf = eati64(&ptr);
    // dom->jorf = eati64(&ptr);

    dom->envsc = eatf32(&ptr);
    dom->domcorrection = eatf32(&ptr);
    dom->dombias = eatf32(&ptr);
    dom->oasc = eatf32(&ptr);
    dom->bitscore = eatf32(&ptr);
    dom->lnP = eatf64(&ptr);
    dom->is_reported = eatu32(&ptr);
    dom->is_included = eatu32(&ptr);

    dom->npos = eatu32(&ptr);

    if (dom->npos > 0)
    {
        size_t size = dom->npos * sizeof(float);
        dom->scores_per_pos = ctb_realloc(dom->scores_per_pos, size);
        if (!dom->scores_per_pos)
        {
            rc = H3C_FAILED_PARSE;
            goto cleanup;
        }

        for (uint64_t i = 0; i < dom->npos; i++)
        {
            dom->scores_per_pos[i] = eatf32(&ptr);
        }
    }

    if (ptr != obj_size + data)
    {
        rc = H3C_FAILED_PARSE;
        goto cleanup;
    }

    size_t size = 0;
    if ((rc = hmmd_alidisplay_parse(&dom->ad, &size, ptr))) goto cleanup;
    ptr += size;

    *read_size = (size_t)(ptr - data);
    return H3C_OK;

cleanup:
    hmmd_domain_cleanup(dom);
    return rc;
}
