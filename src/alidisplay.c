#include "alidisplay.h"
#include "del.h"
#include "h3client/rc.h"
#include "lite_pack/lite_pack.h"
#include <stdlib.h>
#include <string.h>

enum h3c_rc alidisplay_init(struct h3c_alidisplay *ali)
{
    memset(ali, 0, sizeof(*ali));

    if (!(ali->rfline = malloc(1))) goto cleanup;
    if (!(ali->mmline = malloc(1))) goto cleanup;
    if (!(ali->csline = malloc(1))) goto cleanup;
    if (!(ali->model = malloc(1))) goto cleanup;
    if (!(ali->mline = malloc(1))) goto cleanup;
    if (!(ali->aseq = malloc(1))) goto cleanup;
    if (!(ali->ntseq = malloc(1))) goto cleanup;
    if (!(ali->ppline = malloc(1))) goto cleanup;

    if (!(ali->hmmname = malloc(1))) goto cleanup;
    if (!(ali->hmmacc = malloc(1))) goto cleanup;
    if (!(ali->hmmdesc = malloc(1))) goto cleanup;

    if (!(ali->sqname = malloc(1))) goto cleanup;
    if (!(ali->sqacc = malloc(1))) goto cleanup;
    if (!(ali->sqdesc = malloc(1))) goto cleanup;

    return H3C_OK;

cleanup:
    alidisplay_cleanup(ali);
    return H3C_NOT_ENOUGH_MEMORY;
}

void alidisplay_cleanup(struct h3c_alidisplay *ali)
{
    DEL(ali->rfline);
    DEL(ali->mmline);
    DEL(ali->csline);
    DEL(ali->model);
    DEL(ali->mline);
    DEL(ali->aseq);
    DEL(ali->ntseq);
    DEL(ali->ppline);

    DEL(ali->hmmname);
    DEL(ali->hmmacc);
    DEL(ali->hmmdesc);

    DEL(ali->sqname);
    DEL(ali->sqacc);
    DEL(ali->sqdesc);
}

static void write_cstr(struct lip_file *f, char const *str)
{
    if (str)
        lip_write_cstr(f, str);
    else
        lip_write_cstr(f, "");
}

enum h3c_rc alidisplay_pack(struct h3c_alidisplay const *ali,
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

    return lip_file_error(f) ? H3C_FAILED_PACK : H3C_OK;
}
