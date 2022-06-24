#include "hmmd/alidisplay.h"
#include "c_toolbelt/c_toolbelt.h"
#include "h3client/h3client.h"
#include "hmmd/utils.h"
#include "lite_pack/file/file.h"
#include "lite_pack/lite_pack.h"
#include <stdlib.h>
#include <string.h>

void hmmd_alidisplay_init(struct hmmd_alidisplay *ali)
{
    memset(ali, 0, sizeof(*ali));
}

void hmmd_alidisplay_cleanup(struct hmmd_alidisplay *ali)
{
    if (ali->mem) free(ali->mem);
    hmmd_alidisplay_init(ali);
}

#define SER_BASE_SIZE ((5 * sizeof(int)) + (3 * sizeof(int64_t)) + 1)

#define RFLINE_PRESENT (1 << 0)
#define MMLINE_PRESENT (1 << 1)
#define CSLINE_PRESENT (1 << 2)
#define PPLINE_PRESENT (1 << 3)
#define ASEQ_PRESENT (1 << 4)
#define NTSEQ_PRESENT (1 << 5)

enum h3c_rc hmmd_alidisplay_deserialize(struct hmmd_alidisplay *ali,
                                        size_t *read_size,
                                        unsigned char const *data)
{
    enum h3c_rc rc = H3C_OK;
    *read_size = 0;
    unsigned char const *ptr = data;

    size_t obj_size = eatu32(&ptr);
    assert(obj_size > SER_BASE_SIZE);

    if (!(ali->mem = ctb_realloc(ali->mem, obj_size - SER_BASE_SIZE)))
    {
        rc = H3C_NOT_ENOUGH_MEMORY;
        goto cleanup;
    }
    ali->memsize = obj_size - SER_BASE_SIZE;

    ali->N = eatu32(&ptr);
    ali->hmmfrom = eatu32(&ptr);
    ali->hmmto = eatu32(&ptr);
    ali->M = eatu32(&ptr);
    ali->sqfrom = eatu64(&ptr);
    ali->sqto = eatu64(&ptr);
    ali->L = eatu64(&ptr);

    ali->presence = eatu8(&ptr);

    memcpy(ali->mem, ptr, obj_size - SER_BASE_SIZE);
    ptr += obj_size - SER_BASE_SIZE;

    if (ptr != data + obj_size)
    {
        rc = H3C_INVALID_PACK;
        goto cleanup;
    }

    char *mem = ali->mem;
    ali->rfline = ali->presence & RFLINE_PRESENT ? strskip(&mem) : 0;
    ali->mmline = ali->presence & MMLINE_PRESENT ? strskip(&mem) : 0;
    ali->csline = ali->presence & CSLINE_PRESENT ? strskip(&mem) : 0;
    ali->model = strskip(&mem);
    ali->mline = strskip(&mem);
    ali->aseq = ali->presence & ASEQ_PRESENT ? strskip(&mem) : 0;
    ali->ntseq = ali->presence & NTSEQ_PRESENT ? strskip(&mem) : 0;
    ali->ppline = ali->presence & PPLINE_PRESENT ? strskip(&mem) : 0;

    ali->hmmname = strskip(&mem);
    ali->hmmacc = strskip(&mem);
    ali->hmmdesc = strskip(&mem);
    ali->sqname = strskip(&mem);
    ali->sqacc = strskip(&mem);
    ali->sqdesc = strskip(&mem);

    if (mem != ali->mem + obj_size - SER_BASE_SIZE)
    {
        rc = H3C_INVALID_PACK;
        goto cleanup;
    }

    *read_size = (size_t)(ptr - data);
    return H3C_OK;

cleanup:
    hmmd_alidisplay_cleanup(ali);
    return rc;
}

static void write_cstr(struct lip_file *f, char const *str)
{
    if (str)
        lip_write_cstr(f, str);
    else
        lip_write_cstr(f, "");
}

enum h3c_rc hmmd_alidisplay_pack(struct hmmd_alidisplay const *ali,
                                 struct lip_file *f)
{
    lip_write_array_size(f, 19);

    lip_write_int(f, ali->presence);
    write_cstr(f, ali->rfline);
    write_cstr(f, ali->mmline);
    write_cstr(f, ali->csline);
    lip_write_cstr(f, ali->model);
    lip_write_cstr(f, ali->mline);
    write_cstr(f, ali->aseq);
    write_cstr(f, ali->ntseq);
    write_cstr(f, ali->ppline);
    lip_write_int(f, ali->N);

    lip_write_cstr(f, ali->hmmname);
    lip_write_cstr(f, ali->hmmacc);
    lip_write_cstr(f, ali->hmmdesc);
    lip_write_int(f, ali->hmmfrom);
    lip_write_int(f, ali->hmmto);
    lip_write_int(f, ali->M);
    lip_write_int(f, ali->sqfrom);
    lip_write_int(f, ali->sqto);
    lip_write_int(f, ali->L);

    return f->error ? H3C_FAILED_PACK : H3C_OK;
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

static int print_nontranslated(struct hmmd_alidisplay *ad,
                               uint32_t min_aliwidth, uint32_t linewidth,
                               bool show_accessions)
{
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
    if (!buf) goto cleanup;
    buf[aliwidth] = 0;

    /* Break the alignment into multiple blocks of width aliwidth for printing
     */
    i1 = ad->sqfrom;
    k1 = ad->hmmfrom;
    for (pos = 0; pos < ad->N; pos += aliwidth)
    {
        if (pos > 0)
        {
            if (printf("\n") < 0)
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

        if (ad->csline != 0)
        {
            strncpy(buf, ad->csline + pos, aliwidth);
            if (printf("  %*s %s CS\n", namewidth + coordwidth + 1, "", buf) <
                0)
                fprintf(stderr, "alignment display write failed");
        }
        if (ad->rfline != 0)
        {
            strncpy(buf, ad->rfline + pos, aliwidth);
            if (printf("  %*s %s RF\n", namewidth + coordwidth + 1, "", buf) <
                0)
                fprintf(stderr, "alignment display write failed");
        }
        if (ad->mmline != 0)
        {
            strncpy(buf, ad->mmline + pos, aliwidth);
            if (printf("  %*s %s MM\n", namewidth + coordwidth + 1, "", buf) <
                0)
                fprintf(stderr, "alignment display write failed");
        }

        strncpy(buf, ad->model + pos, aliwidth);
        if (printf("  %*s %*d %s %-*d\n", namewidth, show_hmmname, coordwidth,
                   k1, buf, coordwidth, k2) < 0)
            fprintf(stderr, "alignment display write failed");
        strncpy(buf, ad->mline + pos, aliwidth);
        if (printf("  %*s %s\n", namewidth + coordwidth + 1, " ", buf) < 0)
            fprintf(stderr, "alignment display write failed");

        if (ni > 0)
        {
            strncpy(buf, ad->aseq + pos, aliwidth);
            if (printf("  %*s %*ld %s %-*ld\n", namewidth, show_seqname,
                       coordwidth, i1, buf, coordwidth, i2) < 0)
                fprintf(stderr, "alignment display write failed");
        }
        else
        {
            strncpy(buf, ad->aseq + pos, aliwidth);
            if (printf("  %*s %*s %s %*s\n", namewidth, show_seqname,
                       coordwidth, "-", buf, coordwidth, "-") < 0)
                fprintf(stderr, "alignment display write failed");
        }

        if (ad->ppline != 0)
        {
            strncpy(buf, ad->ppline + pos, aliwidth);
            if (printf("  %*s %s PP\n", namewidth + coordwidth + 1, "", buf) <
                0)
                fprintf(stderr, "alignment display write failed");
        }

        k1 += nk;
        if (ad->sqfrom < ad->sqto)
            i1 += ni;
        else
            i1 -= ni; // revcomp hit for DNA
    }
    free(buf);
    return 0;

cleanup:
    free(buf);
    return 1;
}

int hmmd_alidisplay_print(struct hmmd_alidisplay *ad, int min_aliwidth,
                          int linewidth, bool show_accessions)
{
    return print_nontranslated(ad, min_aliwidth, linewidth, show_accessions);
}
