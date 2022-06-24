#include "hmmd/tophits.h"
#include "c_toolbelt/c_toolbelt.h"
#include "h3client/rc.h"
#include "hmmd/domain.h"
#include "hmmd/hit.h"
#include "lite_pack/lite_pack.h"
#include <stdlib.h>
#include <string.h>
#include <tgmath.h>

void hmmd_tophits_init(struct hmmd_tophits *th)
{
    memset(th, 0, sizeof(*th));
    th->hit = 0;
    th->unsrt = 0;
    th->is_sorted_by_sortkey = true;
}

enum h3c_rc hmmd_tophits_setup(struct hmmd_tophits *th,
                               unsigned char const *data, uint64_t nhits,
                               uint64_t nreported, uint64_t nincluded)
{
    enum h3c_rc rc = H3C_OK;

    th->hit = 0;
    th->unsrt = 0;

    if (nhits > 0)
    {
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
    }
    else
    {
        free(th->hit);
        free(th->unsrt);
        th->hit = 0;
        th->unsrt = 0;
    }

    th->nhits = nhits;
    th->nreported = nreported;
    th->nincluded = nincluded;
    th->is_sorted_by_seqidx = false;
    th->is_sorted_by_sortkey = true;

    unsigned char const *ptr = data;
    for (uint64_t i = 0; i < nhits; ++i)
    {
        hmmd_hit_init(th->unsrt + i);
        size_t size = 0;
        if ((rc = hmmd_hit_deserialize(th->unsrt + i, &size, ptr)))
            goto cleanup;
        ptr += size;
        th->hit[i] = th->unsrt + i;
    }

    return H3C_OK;

cleanup:
    hmmd_tophits_cleanup(th);
    return rc;
}

void hmmd_tophits_cleanup(struct hmmd_tophits *th)
{
    for (uint64_t i = 0; i < th->nhits; ++i)
    {
        hmmd_hit_cleanup(th->unsrt + i);
    }
    free(th->hit);
    free(th->unsrt);
    hmmd_tophits_init(th);
}

enum h3c_rc hmmd_tophits_pack(struct hmmd_tophits const *th, struct lip_file *f)
{
    lip_write_array_size(f, 6);
    lip_write_int(f, th->nhits);
    lip_write_int(f, th->nreported);
    lip_write_int(f, th->nincluded);
    lip_write_bool(f, th->is_sorted_by_sortkey);
    lip_write_bool(f, th->is_sorted_by_seqidx);

    lip_write_map_size(f, 1);
    lip_write_cstr(f, "hits");
    lip_write_array_size(f, th->nhits);
    for (uint64_t i = 0; i < th->nhits; ++i)
    {
        enum h3c_rc rc = hmmd_hit_pack(th->unsrt + i, f);
        if (rc) return rc;
    }

    return f->error ? H3C_FAILED_PACK : H3C_OK;
}

#define ESL_MAX(a, b) (((a) > (b)) ? (a) : (b))

uint64_t GetMaxShownLength(struct hmmd_tophits const *h)
{
    uint64_t max = 0;
    uint64_t i = 0;
    uint64_t n = 0;
    for (max = 0, i = 0; i < h->nhits; i++)
    {
        if (h->unsrt[i].acc != 0 && h->unsrt[i].acc[0] != '\0')
        {
            n = strlen(h->unsrt[i].acc);
            max = ESL_MAX(n, max);
        }
        else if (h->unsrt[i].name != 0)
        {
            n = strlen(h->unsrt[i].name);
            max = ESL_MAX(n, max);
        }
    }
    return max;
}

uint64_t GetMaxNameLength(struct hmmd_tophits const *h)
{
    uint64_t max = 0;
    uint64_t i = 0;
    uint64_t n = 0;
    for (max = 0, i = 0; i < h->nhits; i++)
        if (h->unsrt[i].name != 0)
        {
            n = strlen(h->unsrt[i].name);
            max = ESL_MAX(n, max);
        }
    return max;
}

#define p7_HITFLAGS_DEFAULT 0
#define p7_IS_INCLUDED (1 << 0)
#define p7_IS_REPORTED (1 << 1)
#define p7_IS_NEW (1 << 2)
#define p7_IS_DROPPED (1 << 3)
#define p7_IS_DUPLICATE (1 << 4)

enum p7_pipemodes_e
{
    p7_SEARCH_SEQS = 0,
    p7_SCAN_MODELS = 1
};

#define eslCONST_LOG2R 1.44269504088896341

enum h3c_rc hmmd_tophits_print(struct hmmd_tophits const *th,
                               bool show_accessions, double Z)
{
    char newness;
    int h;
    int d;
    int namew;
    int descw;
    char *showname;
    int textw = 120;
    int mode = p7_SCAN_MODELS;

    int have_printed_incthresh = false;

    /* when --acc is on, we'll show accession if available, and fall back to
     * name */
    if (show_accessions)
        namew = ESL_MAX(8, GetMaxShownLength(th));
    else
        namew = ESL_MAX(8, GetMaxNameLength(th));

    if (textw > 0)
        descw = ESL_MAX(32, textw - namew -
                                61); /* 61 chars excluding desc is from the
                                        format: 2 + 22+2 +22+2 +8+2 +<name>+1 */
    else
        descw = 0; /* unlimited desc length is handled separately */

    /* The minimum width of the target table is 111 char: 47 from fields, 8 from
     * min name, 32 from min desc, 13 spaces */
    if (printf("Scores for complete sequence%s (score includes all domains):\n",
               mode == p7_SEARCH_SEQS ? "s" : "") < 0)
        fprintf(stderr, "per-sequence hit list: write failed");
    if (printf("  %22s  %22s  %8s\n", " --- full sequence ---",
               " --- best 1 domain ---", "-#dom-") < 0)
        fprintf(stderr, "per-sequence hit list: write failed");
    if (printf("  %9s %6s %5s  %9s %6s %5s  %5s %2s  %-*s %s\n", "E-value",
               " score", " bias", "E-value", " score", " bias", "  exp", "N",
               namew, (mode == p7_SEARCH_SEQS ? "Sequence" : "Model"),
               "Description") < 0)
        fprintf(stderr, "per-sequence hit list: write failed");
    if (printf("  %9s %6s %5s  %9s %6s %5s  %5s %2s  %-*s %s\n", "-------",
               "------", "-----", "-------", "------", "-----", " ----", "--",
               namew, "--------", "-----------") < 0)
        fprintf(stderr, "per-sequence hit list: write failed");

    for (h = 0; h < th->nhits; h++)
    {
        if (th->hit[h]->flags & p7_IS_REPORTED)
        {
            d = th->hit[h]->best_domain;

            if (!(th->hit[h]->flags & p7_IS_INCLUDED) &&
                !have_printed_incthresh)
            {
                if (printf("  ------ inclusion threshold ------\n") < 0)
                    fprintf(stderr, "per-sequence hit list: write failed");
                have_printed_incthresh = false;
            }

            if (show_accessions)
            { /* the --acc option: report accessions rather than names if
                 possible */
                if (th->hit[h]->acc != 0 && th->hit[h]->acc[0] != '\0')
                    showname = th->hit[h]->acc;
                else
                    showname = th->hit[h]->name;
            }
            else
                showname = th->hit[h]->name;

            if (th->hit[h]->flags & p7_IS_NEW)
                newness = '+';
            else if (th->hit[h]->flags & p7_IS_DROPPED)
                newness = '-';
            else
                newness = ' ';

            if (printf(
                    "%c %9.2g %6.1f %5.1f  %9.2g %6.1f %5.1f  %5.1f %2d  %-*s ",
                    newness, exp(th->hit[h]->lnP) * Z, th->hit[h]->score,
                    th->hit[h]->pre_score -
                        th->hit[h]->score, /* bias correction */
                    exp(th->hit[h]->dcl[d].lnP) * Z,
                    th->hit[h]->dcl[d].bitscore,
                    eslCONST_LOG2R *
                        th->hit[h]->dcl[d].dombias, /* convert NATS to BITS at
                                                       last moment */
                    th->hit[h]->nexpected, th->hit[h]->nreported, namew,
                    showname) < 0)
                fprintf(stderr, "per-sequence hit list: write failed");

            if (textw > 0)
            {
                if (printf(" %-.*s\n", descw,
                           th->hit[h]->desc == 0 ? "" : th->hit[h]->desc) < 0)
                    fprintf(stderr, "per-sequence hit list: write failed");
            }
            else
            {
                if (printf(" %s\n",
                           th->hit[h]->desc == 0 ? "" : th->hit[h]->desc) < 0)
                    fprintf(stderr, "per-sequence hit list: write failed");
            }
            /* do NOT use *s with unlimited (INT_MAX) line length. Some systems
             * have an printf() bug here (we found one on an Opteron/SUSE Linux
             * system (#h66)
             */
        }
    }

    if (th->nreported == 0)
    {
        if (printf(
                "\n   [No hits detected that satisfy reporting thresholds]\n") <
            0)
            fprintf(stderr, "per-sequence hit list: write failed");
    }
    return H3C_OK;
}
