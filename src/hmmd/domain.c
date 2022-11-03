#include "hmmd/domain.h"
#include "h3c/rc.h"
#include "hmmd/alidisplay.h"
#include "utils.h"
#include <assert.h>
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
    if (dom->pos_score) free(dom->pos_score);
    hmmd_alidisplay_cleanup(&dom->ad);
    hmmd_domain_init(dom);
}

static_assert(sizeof(float) == 4, "sizeof(float) == 4");
static_assert(sizeof(double) == 8, "sizeof(double) == 8");

enum h3c_rc hmmd_domain_parse(struct hmmd_domain *dom,
                              unsigned char const **ptr,
                              unsigned char const *end)
{
    enum h3c_rc rc = H3C_OK;

    ESCAPE_OVERRUN(rc, *ptr, end, sizeof(uint32_t) + 6 * sizeof(uint64_t));

    // Skips object size
    (void)eatu32(ptr);
    dom->ienv = eatu64(ptr);
    dom->jenv = eatu64(ptr);
    dom->iali = eatu64(ptr);
    dom->jali = eatu64(ptr);

    // Skips iorf and jorf.
    (void)eati64(ptr);
    (void)eati64(ptr);

    ESCAPE_OVERRUN(rc, *ptr, end, 5 * sizeof(float) + sizeof(double));
    ESCAPE_OVERRUN(rc, *ptr, end, 3 * sizeof(uint32_t));

    dom->envsc = eatf32(ptr);
    dom->domcorrection = eatf32(ptr);
    dom->dombias = eatf32(ptr);
    dom->oasc = eatf32(ptr);
    dom->bitscore = eatf32(ptr);
    dom->lnP = eatf64(ptr);
    dom->is_reported = eatu32(ptr);
    dom->is_included = eatu32(ptr);

    uint32_t scores_size = eatu32(ptr);

    if (scores_size > dom->scores_size)
    {
        size_t size = scores_size * sizeof(float);
        float *scores = realloc(dom->pos_score, size);
        if (!scores)
        {
            rc = H3C_FAILED_PARSE;
            goto cleanup;
        }
        dom->pos_score = scores;
        dom->scores_size = scores_size;
    }
    else
        dom->scores_size = scores_size;

    ESCAPE_OVERRUN(rc, *ptr, end, dom->scores_size * sizeof(float));
    for (uint32_t i = 0; i < dom->scores_size; i++)
        dom->pos_score[i] = eatf32(ptr);

    if ((rc = hmmd_alidisplay_parse(&dom->ad, ptr, end))) goto cleanup;

    return H3C_OK;

cleanup:
    hmmd_domain_cleanup(dom);
    return rc;
}
