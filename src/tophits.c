#include "tophits.h"
#include "del.h"
#include "domain.h"
#include "h3client/rc.h"
#include "hit.h"
#include "lite_pack/lite_pack.h"
#include "tophits.h"
#include <inttypes.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <tgmath.h>

#define eslCONST_LOG2R 1.44269504088896341

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

#define ESL_MAX(a, b) (((a) > (b)) ? (a) : (b))

uint64_t GetMaxShownLength(struct h3c_tophits const *h)
{
    uint64_t max = 0;
    uint64_t i = 0;
    uint64_t n = 0;
    for (max = 0, i = 0; i < h->nhits; i++)
    {
        if (h->hits[i].acc != 0 && h->hits[i].acc[0] != '\0')
        {
            n = strlen(h->hits[i].acc);
            max = ESL_MAX(n, max);
        }
        else if (h->hits[i].name != 0)
        {
            n = strlen(h->hits[i].name);
            max = ESL_MAX(n, max);
        }
    }
    return max;
}

uint64_t GetMaxNameLength(struct h3c_tophits const *h)
{
    uint64_t max = 0;
    uint64_t i = 0;
    uint64_t n = 0;
    for (max = 0, i = 0; i < h->nhits; i++)
    {
        if (h->hits[i].name != 0)
        {
            n = strlen(h->hits[i].name);
            max = ESL_MAX(n, max);
        }
    }
    return max;
}

#define p7_HITFLAGS_DEFAULT 0
#define p7_IS_INCLUDED (1 << 0)
#define p7_IS_REPORTED (1 << 1)
#define p7_IS_NEW (1 << 2)
#define p7_IS_DROPPED (1 << 3)
#define p7_IS_DUPLICATE (1 << 4)

void tophits_print_targets(struct h3c_tophits const *th, FILE *file, double Z)
{
    char newness;
    uint32_t h;
    int d;
    int namew;
    int descw;
    char *showname;
    int textw = 120;
    bool show_accessions = true;

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
    if (fprintf(
            file,
            "Scores for complete sequence%s (score includes all domains):\n",
            "") < 0)
        fprintf(stderr, "per-sequence hit list: write failed");
    if (fprintf(file, "  %22s  %22s  %8s\n", " --- full sequence ---",
                " --- best 1 domain ---", "-#dom-") < 0)
        fprintf(stderr, "per-sequence hit list: write failed");
    if (fprintf(file, "  %9s %6s %5s  %9s %6s %5s  %5s %2s  %-*s %s\n",
                "E-value", " score", " bias", "E-value", " score", " bias",
                "  exp", "N", namew, "Model", "Description") < 0)
        fprintf(stderr, "per-sequence hit list: write failed");
    if (fprintf(file, "  %9s %6s %5s  %9s %6s %5s  %5s %2s  %-*s %s\n",
                "-------", "------", "-----", "-------", "------", "-----",
                " ----", "--", namew, "--------", "-----------") < 0)
        fprintf(stderr, "per-sequence hit list: write failed");

    for (h = 0; h < th->nhits; h++)
    {
        if (th->hits[h].flags & p7_IS_REPORTED)
        {
            d = th->hits[h].best_domain;

            if (!(th->hits[h].flags & p7_IS_INCLUDED) &&
                !have_printed_incthresh)
            {
                if (fprintf(file, "  ------ inclusion threshold ------\n") < 0)
                    fprintf(stderr, "per-sequence hit list: write failed");
                have_printed_incthresh = false;
            }

            if (show_accessions)
            { /* the --acc option: report accessions rather than names if
                 possible */
                if (th->hits[h].acc != 0 && th->hits[h].acc[0] != '\0')
                    showname = th->hits[h].acc;
                else
                    showname = th->hits[h].name;
            }
            else
                showname = th->hits[h].name;

            if (th->hits[h].flags & p7_IS_NEW)
                newness = '+';
            else if (th->hits[h].flags & p7_IS_DROPPED)
                newness = '-';
            else
                newness = ' ';

            if (fprintf(
                    file,
                    "%c %9.2g %6.1f %5.1f  %9.2g %6.1f %5.1f  %5.1f %2d  %-*s ",
                    newness, exp(th->hits[h].lnP) * Z, th->hits[h].score,
                    th->hits[h].pre_score -
                        th->hits[h].score, /* bias correction */
                    exp(th->hits[h].domains[d].lnP) * Z,
                    th->hits[h].domains[d].bitscore,
                    eslCONST_LOG2R *
                        th->hits[h].domains[d].dombias, /* convert NATS to BITS
                                                       at last moment */
                    th->hits[h].nexpected, th->hits[h].nreported, namew,
                    showname) < 0)
                fprintf(stderr, "per-sequence hit list: write failed");

            if (textw > 0)
            {
                if (fprintf(file, " %-.*s\n", descw,
                            th->hits[h].desc == 0 ? "" : th->hits[h].desc) < 0)
                    fprintf(stderr, "per-sequence hit list: write failed");
            }
            else
            {
                if (fprintf(file, " %s\n",
                            th->hits[h].desc == 0 ? "" : th->hits[h].desc) < 0)
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
}

#define p7_HITFLAGS_DEFAULT 0
#define p7_IS_INCLUDED (1 << 0)
#define p7_IS_REPORTED (1 << 1)
#define p7_IS_NEW (1 << 2)
#define p7_IS_DROPPED (1 << 3)
#define p7_IS_DUPLICATE (1 << 4)

void tophits_print_domains(struct h3c_tophits const *th, FILE *file, double Z,
                           double domZ)
{
    bool show_accessions = true;
    bool show_alignments = true;
    uint64_t h = 0;
    uint32_t d = 0;
    int nd;
    int namew, descw;
    int textw = 120;
    char *showname;

    if (fprintf(file, "Domain annotation for each %s%s:\n", "model",
                show_alignments ? " (and alignments)" : "") < 0)
        fprintf(stderr, "domain hit list: write failed");

    for (h = 0; h < th->nhits; h++)
    {
        if (th->hits[h].flags & p7_IS_REPORTED)
        {
            if (show_accessions && th->hits[h].acc != NULL &&
                th->hits[h].acc[0] != '\0')
            {
                showname = th->hits[h].acc;
                namew = strlen(th->hits[h].acc);
            }
            else
            {
                showname = th->hits[h].name;
                namew = strlen(th->hits[h].name);
            }

            if (textw > 0)
            {
                descw = ESL_MAX(32, textw - namew - 5);
                if (fprintf(
                        file, ">> %s  %-.*s\n", showname, descw,
                        (th->hits[h].desc == NULL ? "" : th->hits[h].desc)) < 0)
                    fprintf(stderr, "domain hit list: write failed");
            }
            else
            {
                if (fprintf(
                        file, ">> %s  %s\n", showname,
                        (th->hits[h].desc == NULL ? "" : th->hits[h].desc)) < 0)
                    fprintf(stderr, "domain hit list: write failed");
            }

            if (th->hits[h].nreported == 0)
            {
                if (fprintf(file,
                            "   [No individual domains that satisfy reporting "
                            "thresholds (although complete target did)]\n\n") <
                    0)
                    fprintf(stderr, "domain hit list: write failed");
                continue;
            }

            /* The domain table is 101 char wide:
                    #     score  bias  c-Evalue  i-Evalue hmmfrom   hmmto
               alifrom  ali to    envfrom  env to     acc
                   ---   ------ ----- --------- --------- ------- -------
               ------- -------    ------- -------    ---- 1 ?
               123.4  23.1   9.7e-11    6.8e-9       3    1230 ..       1 492 []
               2     490 .] 0.90 123 ! 1234.5 123.4 123456789 123456789 1234567
               1234567 .. 1234567 1234567 [] 1234567 1234568 .] 0.12
            */
            if (fprintf(
                    file,
                    " %3s   %6s %5s %9s %9s %7s %7s %2s %7s %7s %2s %7s %7s "
                    "%2s %4s\n",
                    "#", "score", "bias", "c-Evalue", "i-Evalue", "hmmfrom",
                    "hmm to", "  ", "alifrom", "ali to", "  ", "envfrom",
                    "env to", "  ", "acc") < 0)
                fprintf(stderr, "domain hit list: write failed");
            if (fprintf(
                    file,
                    " %3s   %6s %5s %9s %9s %7s %7s %2s %7s %7s %2s %7s %7s "
                    "%2s %4s\n",
                    "---", "------", "-----", "---------", "---------",
                    "-------", "-------", "  ", "-------", "-------", "  ",
                    "-------", "-------", "  ", "----") < 0)
                fprintf(stderr, "domain hit list: write failed");

            /* Domain hit table for each reported domain in this reported
             * sequence. */
            nd = 0;
            for (d = 0; d < th->hits[h].ndomains; d++)
            {
                if (th->hits[h].domains[d].is_reported)
                {
                    nd++;
                    if (fprintf(file,
                                " %3d %c %6.1f %5.1f %9.2g %9.2g %7d %7d %c%c",
                                nd,
                                th->hits[h].domains[d].is_included ? '!' : '?',
                                th->hits[h].domains[d].bitscore,
                                th->hits[h].domains[d].dombias *
                                    eslCONST_LOG2R, /* convert NATS to BITS at
                                                       last moment */
                                exp(th->hits[h].domains[d].lnP) * domZ,
                                exp(th->hits[h].domains[d].lnP) * Z,
                                th->hits[h].domains[d].ad.hmmfrom,
                                th->hits[h].domains[d].ad.hmmto,
                                (th->hits[h].domains[d].ad.hmmfrom == 1) ? '['
                                                                         : '.',
                                (th->hits[h].domains[d].ad.hmmto ==
                                 th->hits[h].domains[d].ad.M)
                                    ? ']'
                                    : '.') < 0)
                        fprintf(stderr, "domain hit list: write failed");

                    if (fprintf(file, " %7" PRId64 " %7" PRId64 " %c%c",
                                th->hits[h].domains[d].ad.sqfrom,
                                th->hits[h].domains[d].ad.sqto,
                                (th->hits[h].domains[d].ad.sqfrom == 1) ? '['
                                                                        : '.',
                                (th->hits[h].domains[d].ad.sqto ==
                                 th->hits[h].domains[d].ad.L)
                                    ? ']'
                                    : '.') < 0)
                        fprintf(stderr, "domain hit list: write failed");

                    if (fprintf(file, " %7" PRId64 " %7" PRId64 " %c%c",
                                th->hits[h].domains[d].ienv,
                                th->hits[h].domains[d].jenv,
                                (th->hits[h].domains[d].ienv == 1) ? '[' : '.',
                                (th->hits[h].domains[d].jenv ==
                                 th->hits[h].domains[d].ad.L)
                                    ? ']'
                                    : '.') < 0)
                        fprintf(stderr, "domain hit list: write failed");

                    if (fprintf(
                            file, " %4.2f\n",
                            (th->hits[h].domains[d].oasc /
                             (1.0 +
                              fabs((float)(th->hits[h].domains[d].jenv -
                                           th->hits[h].domains[d].ienv))))) < 0)
                        fprintf(stderr, "domain hit list: write failed");
                }
            } // end of domain table in this reported sequence.

            /* Alignment data for each reported domain in this reported
             * sequence. */
            if (show_alignments)
            {

                if (fprintf(file, "\n  Alignments for each domain:\n") < 0)
                    fprintf(stderr, "domain hit list: write failed");
                nd = 0;

                for (d = 0; d < th->hits[h].ndomains; d++)
                {
                    if (th->hits[h].domains[d].is_reported)
                    {
                        nd++;
                        fprintf(file, "  == domain %d", nd);
                        if (fprintf(file, "  score: %.1f bits",
                                    th->hits[h].domains[d].bitscore) < 0)
                            fprintf(stderr, "domain hit list: write failed");

                        fprintf(file, ";  conditional E-value: %.2g\n",
                                exp(th->hits[h].domains[d].lnP) * domZ);

                        alidisplay_print(&th->hits[h].domains[d].ad, file);

                        if (fprintf(file, "\n") < 0)
                            fprintf(stderr, "domain hit list: write failed");
                    }
                }
            }
            else // alignment reporting is off:
            {
                if (fprintf(file, "\n") < 0)
                    fprintf(stderr, "domain hit list: write failed");
            }

        } // end, loop over all reported hits
    }

    if (th->nreported == 0)
    {
        if (fprintf(file, "\n   [No targets detected that satisfy reporting "
                          "thresholds]\n") < 0)
            fprintf(stderr, "domain hit list: write failed");
    }
}

static int GetMaxAccessionLength(struct h3c_tophits const *th)
{
    uint32_t i, max, n;
    for (max = 0, i = 0; i < th->nhits; i++)
    {
        if (th->hits[i].acc != NULL)
        {
            n = strlen(th->hits[i].acc);
            max = ESL_MAX(n, max);
        }
    }
    return max;
}

void tophits_print_tabular_targets(char *qname, char *qacc,
                                   struct h3c_tophits const *th,
                                   int show_header, double Z)
{
    int qnamew = ESL_MAX(20, strlen(qname));
    int tnamew = ESL_MAX(20, GetMaxNameLength(th));
    int qaccw = ((qacc != NULL) ? ESL_MAX(10, strlen(qacc)) : 10);
    int taccw = ESL_MAX(10, GetMaxAccessionLength(th));
    uint32_t h, d;

    if (show_header)
    {
        if (printf("#%*s %22s %22s %33s\n", tnamew + qnamew + taccw + qaccw + 2,
                   "", "--- full sequence ----", "--- best 1 domain ----",
                   "--- domain number estimation ----") < 0)
            fprintf(stderr, "tabular per-sequence hit list: write failed");
        if (printf("#%-*s %-*s %-*s %-*s %9s %6s %5s %9s %6s %5s %5s %3s %3s "
                   "%3s %3s %3s %3s %3s %s\n",
                   tnamew - 1, " target name", taccw, "accession", qnamew,
                   "query name", qaccw, "accession", "  E-value", " score",
                   " bias", "  E-value", " score", " bias", "exp", "reg", "clu",
                   " ov", "env", "dom", "rep", "inc",
                   "description of target") < 0)
            fprintf(stderr, "tabular per-sequence hit list: write failed");
        if (printf("#%*s %*s %*s %*s %9s %6s %5s %9s %6s %5s %5s %3s %3s %3s "
                   "%3s %3s %3s %3s %s\n",
                   tnamew - 1, "-------------------", taccw, "----------",
                   qnamew, "--------------------", qaccw, "----------",
                   "---------", "------", "-----", "---------", "------",
                   "-----", "---", "---", "---", "---", "---", "---", "---",
                   "---", "---------------------") < 0)
            fprintf(stderr, "tabular per-sequence hit list: write failed");
    }

    for (h = 0; h < th->nhits; h++)
    {
        if (th->hits[h].flags & p7_IS_REPORTED)
        {
            d = th->hits[h].best_domain;
            if (printf("%-*s %-*s %-*s %-*s %9.2g %6.1f %5.1f %9.2g %6.1f "
                       "%5.1f %5.1f %3d %3d %3d %3d %3d %3d %3d %s\n",
                       tnamew, th->hits[h].name, taccw,
                       th->hits[h].acc ? th->hits[h].acc : "-", qnamew, qname,
                       qaccw, ((qacc != NULL && qacc[0] != '\0') ? qacc : "-"),
                       exp(th->hits[h].lnP) * Z, th->hits[h].score,
                       th->hits[h].pre_score -
                           th->hits[h].score, /* bias correction */
                       exp(th->hits[h].domains[d].lnP) * Z,
                       th->hits[h].domains[d].bitscore,
                       th->hits[h].domains[d].dombias *
                           eslCONST_LOG2R, /* convert NATS to BITS at last
                                              moment */
                       th->hits[h].nexpected, th->hits[h].nregions,
                       th->hits[h].nclustered, th->hits[h].noverlaps,
                       th->hits[h].nenvelopes, th->hits[h].ndomains,
                       th->hits[h].nreported, th->hits[h].nincluded,
                       (th->hits[h].desc == NULL ? "-" : th->hits[h].desc)) < 0)
                fprintf(stderr, "tabular per-sequence hit list: write failed");
        }
    }
}

void tophits_print_tabular_domains(char *qname, char *qacc,
                                   struct h3c_tophits const *th,
                                   int show_header, double Z, double domZ)
{
    int qnamew = ESL_MAX(20, strlen(qname));
    int tnamew = ESL_MAX(20, GetMaxNameLength(th));
    int qaccw = (qacc ? ESL_MAX(10, strlen(qacc)) : 10);
    int taccw = ESL_MAX(10, GetMaxAccessionLength(th));
    int tlen, qlen;
    uint32_t h, d, nd;

    if (show_header)
    {
        if (printf("#%*s %22s %40s %11s %11s %11s\n",
                   tnamew + qnamew - 1 + 15 + taccw + qaccw, "",
                   "--- full sequence ---",
                   "-------------- this domain -------------", "hmm coord",
                   "ali coord", "env coord") < 0)
            fprintf(stderr, "tabular per-domain hit list: write failed");
        if (printf("#%-*s %-*s %5s %-*s %-*s %5s %9s %6s %5s %3s %3s %9s %9s "
                   "%6s %5s %5s %5s %5s %5s %5s %5s %4s %s\n",
                   tnamew - 1, " target name", taccw, "accession", "tlen",
                   qnamew, "query name", qaccw, "accession", "qlen", "E-value",
                   "score", "bias", "#", "of", "c-Evalue", "i-Evalue", "score",
                   "bias", "from", "to", "from", "to", "from", "to", "acc",
                   "description of target") < 0)
            fprintf(stderr, "tabular per-domain hit list: write failed");
        if (printf("#%*s %*s %5s %*s %*s %5s %9s %6s %5s %3s %3s %9s %9s %6s "
                   "%5s %5s %5s %5s %5s %5s %5s %4s %s\n",
                   tnamew - 1, "-------------------", taccw, "----------",
                   "-----", qnamew, "--------------------", qaccw, "----------",
                   "-----", "---------", "------", "-----", "---", "---",
                   "---------", "---------", "------", "-----", "-----",
                   "-----", "-----", "-----", "-----", "-----", "----",
                   "---------------------") < 0)
            fprintf(stderr, "tabular per-domain hit list: write failed");
    }

    for (h = 0; h < th->nhits; h++)
    {
        if (th->hits[h].flags & p7_IS_REPORTED)
        {
            nd = 0;
            for (d = 0; d < th->hits[h].ndomains; d++)
            {
                if (th->hits[h].domains[d].is_reported)
                {
                    nd++;

                    qlen = th->hits[h].domains[d].ad.L;
                    tlen = th->hits[h].domains[d].ad.M;

                    if (printf(
                            "%-*s %-*s %5d %-*s %-*s %5d %9.2g %6.1f %5.1f %3d "
                            "%3d %9.2g %9.2g %6.1f %5.1f %5d %5d %5" PRId64
                            " %5" PRId64 " %5" PRId64 " %5" PRId64
                            " %4.2f %s\n",
                            tnamew, th->hits[h].name, taccw,
                            th->hits[h].acc ? th->hits[h].acc : "-", tlen,
                            qnamew, qname, qaccw,
                            ((qacc != NULL && qacc[0] != '\0') ? qacc : "-"),
                            qlen, exp(th->hits[h].lnP) * Z, th->hits[h].score,
                            th->hits[h].pre_score -
                                th->hits[h].score, /* bias correction */
                            nd, th->hits[h].nreported,
                            exp(th->hits[h].domains[d].lnP) * domZ,
                            exp(th->hits[h].domains[d].lnP) * Z,
                            th->hits[h].domains[d].bitscore,
                            th->hits[h].domains[d].dombias *
                                eslCONST_LOG2R, /* NATS to BITS at last moment
                                                 */
                            th->hits[h].domains[d].ad.hmmfrom,
                            th->hits[h].domains[d].ad.hmmto,
                            th->hits[h].domains[d].ad.sqfrom,
                            th->hits[h].domains[d].ad.sqto,
                            th->hits[h].domains[d].ienv,
                            th->hits[h].domains[d].jenv,
                            (th->hits[h].domains[d].oasc /
                             (1.0 +
                              fabs((float)(th->hits[h].domains[d].jenv -
                                           th->hits[h].domains[d].ienv)))),
                            (th->hits[h].desc ? th->hits[h].desc : "-")) < 0)
                        fprintf(stderr,
                                "tabular per-domain hit list: write failed");
                }
            }
        }
    }
}
