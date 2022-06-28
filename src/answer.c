#include "answer.h"
#include "buff.h"
#include "c_toolbelt/c_toolbelt.h"
#include "domain.h"
#include "h3client/rc.h"
#include "h3client/result.h"
#include "hit.h"
#include "hmmd/hmmd.h"
#include "result.h"
#include "stats.h"
#include "strxdup.h"
#include "tophits.h"
#include <stdlib.h>
#include <string.h>

#define BUFF_SIZE 2048

struct answer
{
    struct
    {
        unsigned char data[HMMD_STATUS_PACK_SIZE];
        struct hmmd_status value;
    } status;

    struct buff *buff;
    struct hmmd_stats stats;
    struct hmmd_tophits tophits;
};

struct answer *answer_new(void)
{
    struct answer *ans = malloc(sizeof(*ans));
    if (!ans) return 0;

    ans->status.data[0] = '\0';

    if (!(ans->buff = buff_new(BUFF_SIZE)))
    {
        free(ans);
        return 0;
    }
    hmmd_stats_init(&ans->stats);
    hmmd_tophits_init(&ans->tophits);
    return ans;
}

void answer_del(struct answer const *ans)
{
    buff_del(ans->buff);
    hmmd_stats_cleanup((struct hmmd_stats *)&ans->stats);
    hmmd_tophits_cleanup((struct hmmd_tophits *)&ans->tophits);
    free((void *)ans);
}

unsigned char *answer_status_data(struct answer *ans)
{
    return ans->status.data;
}

size_t answer_status_size(void) { return HMMD_STATUS_PACK_SIZE; }

struct hmmd_status const *answer_status_parse(struct answer *ans)
{
    size_t size = 0;
    hmmd_status_parse(&ans->status.value, &size, ans->status.data);
    return &ans->status.value;
}

enum h3c_rc answer_ensure(struct answer *ans, size_t size)
{
    return buff_ensure(&ans->buff, size);
}

unsigned char *answer_data(struct answer *ans) { return ans->buff->data; }

enum h3c_rc answer_parse(struct answer *ans)
{
    size_t read_size = 0;
    enum h3c_rc rc = H3C_OK;
    if ((rc = hmmd_stats_parse(&ans->stats, &read_size, ans->buff->data)))
        goto cleanup;

    rc = hmmd_tophits_setup(&ans->tophits, ans->buff->data + read_size,
                            ans->stats.nhits, ans->stats.nreported,
                            ans->stats.nincluded);
    return H3C_OK;

cleanup:
    return rc;
}

#define STRXDUP(D, S) (D = strxdup((D), (S)))

static enum h3c_rc copy_alidisplay(struct h3c_alidisplay *dst,
                                   struct hmmd_alidisplay const *src)
{
    dst->presence = src->presence;

    if (!STRXDUP(dst->rfline, src->rfline)) goto cleanup;
    if (!STRXDUP(dst->mmline, src->mmline)) goto cleanup;
    if (!STRXDUP(dst->csline, src->csline)) goto cleanup;
    if (!STRXDUP(dst->model, src->model)) goto cleanup;
    if (!STRXDUP(dst->mline, src->mline)) goto cleanup;
    if (!STRXDUP(dst->aseq, src->aseq)) goto cleanup;
    if (!STRXDUP(dst->ntseq, src->ntseq)) goto cleanup;
    if (!STRXDUP(dst->ppline, src->ppline)) goto cleanup;
    dst->N = src->N;

    if (!STRXDUP(dst->hmmname, src->hmmname)) goto cleanup;
    if (!STRXDUP(dst->hmmacc, src->hmmacc)) goto cleanup;
    if (!STRXDUP(dst->hmmdesc, src->hmmdesc)) goto cleanup;
    dst->hmmfrom = src->hmmfrom;
    dst->hmmto = src->hmmto;
    dst->M = src->M;

    if (!STRXDUP(dst->sqname, src->sqname)) goto cleanup;
    if (!STRXDUP(dst->sqacc, src->sqacc)) goto cleanup;
    if (!STRXDUP(dst->sqdesc, src->sqdesc)) goto cleanup;
    dst->sqfrom = src->sqfrom;
    dst->sqto = src->sqto;
    dst->L = src->L;

    return H3C_OK;

cleanup:
    alidisplay_cleanup(dst);
    return H3C_NOT_ENOUGH_MEMORY;
}

static enum h3c_rc copy_domain(struct h3c_domain *dst,
                               struct hmmd_domain const *src)
{
    enum h3c_rc rc = domain_setup(dst, src->npos);
    if (rc) return rc;

    dst->ienv = src->ienv;
    dst->jenv = src->jenv;
    dst->iali = src->iali;
    dst->jali = src->jali;
    dst->envsc = src->envsc;
    dst->domcorrection = src->domcorrection;
    dst->dombias = src->dombias;
    dst->oasc = src->oasc;
    dst->bitscore = src->bitscore;
    dst->lnP = src->lnP;
    dst->is_reported = src->is_reported;
    dst->is_included = src->is_included;

    for (uint32_t i = 0; i < dst->npos; ++i)
        dst->scores_per_pos[i] = src->scores_per_pos[i];

    if ((rc = copy_alidisplay(&dst->ad, &src->ad))) goto cleanup;

    return H3C_OK;

cleanup:
    domain_cleanup(dst);
    return rc;
}

static enum h3c_rc copy_hit(struct h3c_hit *dst, struct hmmd_hit const *src)
{
    enum h3c_rc rc = hit_setup(dst, src->ndom);
    if (rc) return rc;

    if (!STRXDUP(dst->name, src->name)) goto cleanup;
    if (!STRXDUP(dst->acc, src->acc)) goto cleanup;
    if (!STRXDUP(dst->desc, src->desc)) goto cleanup;

    dst->sortkey = src->sortkey;

    dst->score = src->score;
    dst->pre_score = src->pre_score;
    dst->sum_score = src->sum_score;

    dst->lnP = src->lnP;
    dst->pre_lnP = src->pre_lnP;
    dst->sum_lnP = src->sum_lnP;

    dst->nexpected = src->nexpected;
    dst->nregions = src->nregions;
    dst->nclustered = src->nclustered;
    dst->noverlaps = src->noverlaps;
    dst->nenvelopes = src->nenvelopes;

    dst->flags = src->flags;
    dst->nreported = src->nreported;
    dst->nincluded = src->nincluded;
    dst->best_domain = src->best_domain;

    for (uint32_t i = 0; i < dst->ndomains; ++i)
    {
        if ((rc = copy_domain(dst->domains + i, src->dcl + i))) goto cleanup;
    }

    return H3C_OK;

cleanup:
    hit_cleanup(dst);
    return rc;
}

static enum h3c_rc copy_tophits(struct h3c_tophits *dst,
                                struct hmmd_tophits const *src)
{
    enum h3c_rc rc = tophits_setup(dst, src->nhits);
    if (rc) return rc;

    dst->nreported = src->nreported;
    dst->nincluded = src->nincluded;
    dst->is_sorted_by_sortkey = src->is_sorted_by_sortkey;
    dst->is_sorted_by_seqidx = src->is_sorted_by_seqidx;

    for (uint64_t i = 0; i < src->nhits; ++i)
    {
        if ((rc = copy_hit(dst->hits + i, src->hit[i]))) goto cleanup;
    }

    return H3C_OK;

cleanup:
    tophits_cleanup(dst);
    return rc;
}

static void copy_stats(struct h3c_stats *dst, struct hmmd_stats const *src)
{
    dst->Z = src->Z;
    dst->domZ = src->domZ;

    dst->Z_setby = (enum h3c_zsetby)src->Z_setby;
    dst->domZ_setby = (enum h3c_zsetby)src->domZ_setby;

    dst->nmodels = src->nmodels;
    dst->nseqs = src->nseqs;
    dst->n_past_msv = src->n_past_msv;
    dst->n_past_bias = src->n_past_bias;
    dst->n_past_vit = src->n_past_vit;
    dst->n_past_fwd = src->n_past_fwd;

    dst->nhits = src->nhits;
    dst->nreported = src->nreported;
    dst->nincluded = src->nincluded;
}

enum h3c_rc answer_copy(struct answer *ans, struct h3c_result *r)
{
    copy_stats(&r->stats, &ans->stats);
    return copy_tophits(&r->tophits, &ans->tophits);
}
