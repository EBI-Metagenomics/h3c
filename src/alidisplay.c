#include "alidisplay.h"
#include "del.h"
#include "h3client/rc.h"
#include "lite_pack/lite_pack.h"
#include <stdlib.h>
#include <string.h>

#define RFLINE_PRESENT (1 << 0)
#define MMLINE_PRESENT (1 << 1)
#define CSLINE_PRESENT (1 << 2)
#define PPLINE_PRESENT (1 << 3)
#define ASEQ_PRESENT (1 << 4)
#define NTSEQ_PRESENT (1 << 5)

enum h3c_rc alidisplay_init(struct h3c_alidisplay *ad)
{
    memset(ad, 0, sizeof(*ad));

    if (!(ad->rfline = malloc(1))) goto cleanup;
    if (!(ad->mmline = malloc(1))) goto cleanup;
    if (!(ad->csline = malloc(1))) goto cleanup;
    if (!(ad->model = malloc(1))) goto cleanup;
    if (!(ad->mline = malloc(1))) goto cleanup;
    if (!(ad->aseq = malloc(1))) goto cleanup;
    if (!(ad->ntseq = malloc(1))) goto cleanup;
    if (!(ad->ppline = malloc(1))) goto cleanup;

    if (!(ad->hmmname = malloc(1))) goto cleanup;
    if (!(ad->hmmacc = malloc(1))) goto cleanup;
    if (!(ad->hmmdesc = malloc(1))) goto cleanup;

    if (!(ad->sqname = malloc(1))) goto cleanup;
    if (!(ad->sqacc = malloc(1))) goto cleanup;
    if (!(ad->sqdesc = malloc(1))) goto cleanup;

    return H3C_OK;

cleanup:
    alidisplay_cleanup(ad);
    return H3C_NOT_ENOUGH_MEMORY;
}

void alidisplay_cleanup(struct h3c_alidisplay *ad)
{
    DEL(ad->rfline);
    DEL(ad->mmline);
    DEL(ad->csline);
    DEL(ad->model);
    DEL(ad->mline);
    DEL(ad->aseq);
    DEL(ad->ntseq);
    DEL(ad->ppline);

    DEL(ad->hmmname);
    DEL(ad->hmmacc);
    DEL(ad->hmmdesc);

    DEL(ad->sqname);
    DEL(ad->sqacc);
    DEL(ad->sqdesc);
}

static void write_cstr(struct lip_file *f, char const *str)
{
    if (str)
        lip_write_cstr(f, str);
    else
        lip_write_cstr(f, "");
}

enum h3c_rc alidisplay_pack(struct h3c_alidisplay const *ad, struct lip_file *f)
{
    lip_write_array_size(f, 19);

    lip_write_int(f, ad->presence);
    write_cstr(f, ad->rfline);
    write_cstr(f, ad->mmline);
    write_cstr(f, ad->csline);
    lip_write_cstr(f, ad->model);
    lip_write_cstr(f, ad->mline);
    write_cstr(f, ad->aseq);
    write_cstr(f, ad->ntseq);
    write_cstr(f, ad->ppline);
    lip_write_int(f, ad->N);

    lip_write_cstr(f, ad->hmmname);
    lip_write_cstr(f, ad->hmmacc);
    lip_write_cstr(f, ad->hmmdesc);
    lip_write_int(f, ad->hmmfrom);
    lip_write_int(f, ad->hmmto);
    lip_write_int(f, ad->M);
    lip_write_int(f, ad->sqfrom);
    lip_write_int(f, ad->sqto);
    lip_write_int(f, ad->L);

    return lip_file_error(f) ? H3C_FAILED_PACK : H3C_OK;
}

#define ESL_MAX(a, b) (((a) > (b)) ? (a) : (b))

static int integer_textwidth(long n)
{
    int w = (n < 0) ? 1 : 0;
    while (n != 0)
    {
        n /= 10;
        w++;
    }
    return w;
}

void alidisplay_print(struct h3c_alidisplay const *ad, FILE *file)
{
    int min_aliwidth = 40;
    int linewidth = 120;
    bool show_accessions = true;

    char *buf = 0;
    char *show_hmmname = 0;
    char *show_seqname = 0;
    uint32_t namewidth, coordwidth, aliwidth;
    uint32_t pos;
    int ni, nk;
    uint32_t z;
    long i1, i2;
    int k1, k2;

    /* implement the --acc option for preferring accessions over names in output
     */
    show_hmmname =
        (show_accessions && ad->hmmacc[0] != '\0') ? ad->hmmacc : ad->hmmname;
    show_seqname =
        (show_accessions && ad->sqacc[0] != '\0') ? ad->sqacc : ad->sqname;

    /* dynamically size the output lines */
    namewidth = ESL_MAX(strlen(show_hmmname), strlen(show_seqname));
    coordwidth = ESL_MAX(
        ESL_MAX(integer_textwidth(ad->hmmfrom), integer_textwidth(ad->hmmto)),
        ESL_MAX(integer_textwidth(ad->sqfrom), integer_textwidth(ad->sqto)));

    aliwidth =
        (linewidth > 0) ? linewidth - namewidth - 2 * coordwidth - 5 : ad->N;
    if (aliwidth < ad->N && aliwidth < min_aliwidth)
        aliwidth = min_aliwidth; /* at least, regardless of some silly linewidth
                                    setting */
    buf = malloc(sizeof(char) * (aliwidth + 1));
    if (!buf) return;
    buf[aliwidth] = 0;

    /* Break the alignment into multiple blocks of width aliwidth for printing
     */
    i1 = ad->sqfrom;
    k1 = ad->hmmfrom;
    for (pos = 0; pos < ad->N; pos += aliwidth)
    {
        if (pos > 0)
        {
            if (fprintf(file, "\n") < 0)
                fprintf(stderr, "alignment display write failed");
        } /* blank line betweeen blocks */

        ni = nk = 0;
        for (z = pos; z < pos + aliwidth && z < ad->N; z++)
        {
            if (ad->model[z] != '.')
                nk++; /* k advances except on insert states */
            if (ad->aseq[z] != '-')
                ni++; /* i advances except on delete states */
        }

        k2 = k1 + nk - 1;
        if (ad->sqfrom < ad->sqto)
            i2 = i1 + ni - 1;
        else
            i2 = i1 - ni + 1; // revcomp hit for DNA

        if (ad->presence & CSLINE_PRESENT)
        {
            strncpy(buf, ad->csline + pos, aliwidth);
            if (fprintf(file, "  %*s %s CS\n", namewidth + coordwidth + 1, "",
                        buf) < 0)
                fprintf(stderr, "alignment display write failed");
        }
        if (ad->presence & RFLINE_PRESENT)
        {
            strncpy(buf, ad->rfline + pos, aliwidth);
            if (fprintf(file, "  %*s %s RF\n", namewidth + coordwidth + 1, "",
                        buf) < 0)
                fprintf(stderr, "alignment display write failed");
        }
        if (ad->presence & MMLINE_PRESENT)
        {
            strncpy(buf, ad->mmline + pos, aliwidth);
            if (fprintf(file, "  %*s %s MM\n", namewidth + coordwidth + 1, "",
                        buf) < 0)
                fprintf(stderr, "alignment display write failed");
        }

        strncpy(buf, ad->model + pos, aliwidth);
        if (fprintf(file, "  %*s %*d %s %-*d\n", namewidth, show_hmmname,
                    coordwidth, k1, buf, coordwidth, k2) < 0)
            fprintf(stderr, "alignment display write failed");
        strncpy(buf, ad->mline + pos, aliwidth);
        if (fprintf(file, "  %*s %s\n", namewidth + coordwidth + 1, " ", buf) <
            0)
            fprintf(stderr, "alignment display write failed");

        if (ni > 0)
        {
            strncpy(buf, ad->aseq + pos, aliwidth);
            if (fprintf(file, "  %*s %*ld %s %-*ld\n", namewidth, show_seqname,
                        coordwidth, i1, buf, coordwidth, i2) < 0)
                fprintf(stderr, "alignment display write failed");
        }
        else
        {
            strncpy(buf, ad->aseq + pos, aliwidth);
            if (fprintf(file, "  %*s %*s %s %*s\n", namewidth, show_seqname,
                        coordwidth, "-", buf, coordwidth, "-") < 0)
                fprintf(stderr, "alignment display write failed");
        }

        if (ad->ppline != 0)
        {
            strncpy(buf, ad->ppline + pos, aliwidth);
            if (fprintf(file, "  %*s %s PP\n", namewidth + coordwidth + 1, "",
                        buf) < 0)
                fprintf(stderr, "alignment display write failed");
        }

        k1 += nk;
        if (ad->sqfrom < ad->sqto)
            i1 += ni;
        else
            i1 -= ni; // revcomp hit for DNA
    }
    free(buf);
}
