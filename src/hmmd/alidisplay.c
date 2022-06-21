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

enum h3c_rc hmmd_alidisplay_unpack(struct hmmd_alidisplay *ali,
                                   size_t *read_size, unsigned char const *data)
{
    enum h3c_rc rc = H3C_OK;
    *read_size = 0;
    unsigned char const *ptr = data;

    size_t obj_size = eatu32(&ptr);

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

    printf("N: %d\n", ali->N);
    printf("hmmfrom: %d\n", ali->hmmfrom);
    printf("hmmto: %d\n", ali->hmmto);
    printf("M: %d\n", ali->M);
    printf("sqfrom: %lld\n", ali->sqfrom);
    printf("sqto: %lld\n", ali->sqto);
    printf("L: %lld\n", ali->L);

    uint8_t presence = eatu8(&ptr);

    memcpy(ali->mem, ptr, obj_size - SER_BASE_SIZE);
    ptr += obj_size - SER_BASE_SIZE;

    if (ptr != data + obj_size)
    {
        rc = H3C_INVALID_PACK;
        goto cleanup;
    }

    char *mem = ali->mem;
    ali->rfline = presence & RFLINE_PRESENT ? strskip(&mem) : 0;
    ali->mmline = presence & MMLINE_PRESENT ? strskip(&mem) : 0;
    ali->csline = presence & CSLINE_PRESENT ? strskip(&mem) : 0;
    ali->model = strskip(&mem);
    ali->mline = strskip(&mem);
    ali->aseq = presence & ASEQ_PRESENT ? strskip(&mem) : 0;
    ali->ntseq = presence & NTSEQ_PRESENT ? strskip(&mem) : 0;
    ali->ppline = presence & PPLINE_PRESENT ? strskip(&mem) : 0;

    if (presence & RFLINE_PRESENT) printf("rfline: %s\n", ali->rfline);
    if (presence & MMLINE_PRESENT) printf("mmline: %s\n", ali->mmline);
    if (presence & CSLINE_PRESENT) printf("csline: %s\n", ali->csline);

    printf("model: %s\n", ali->model);
    printf("mline: %s\n", ali->mline);

    if (presence & ASEQ_PRESENT) printf("aseq: %s\n", ali->aseq);
    if (presence & NTSEQ_PRESENT) printf("ntseq: %s\n", ali->ntseq);
    if (presence & PPLINE_PRESENT) printf("ppline: %s\n", ali->ppline);

    ali->hmmname = strskip(&mem);
    ali->hmmacc = strskip(&mem);
    ali->hmmdesc = strskip(&mem);
    ali->sqname = strskip(&mem);
    ali->sqacc = strskip(&mem);
    ali->sqdesc = strskip(&mem);

    printf("hmmname: %s\n", ali->hmmname);
    printf("hmmacc: %s\n", ali->hmmacc);
    printf("hmmdesc: %s\n", ali->hmmdesc);
    printf("sqname: %s\n", ali->sqname);
    printf("sqacc: %s\n", ali->sqacc);
    printf("sqdesc: %s\n", ali->sqdesc);

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

enum h3c_rc hmmd_alidisplay_pack(struct hmmd_alidisplay *ali,
                                 struct lip_file *f)
{
    lip_write_cstr(f, ali->rfline);
    lip_write_cstr(f, ali->mmline);
    lip_write_cstr(f, ali->csline);
    lip_write_cstr(f, ali->model);
    lip_write_cstr(f, ali->mline);
    lip_write_cstr(f, ali->aseq);
    lip_write_cstr(f, ali->ntseq);
    lip_write_cstr(f, ali->ppline);
    lip_write_int(f, ali->N);

    lip_write_cstr(f, ali->hmmname);
    lip_write_cstr(f, ali->hmmacc);
    lip_write_cstr(f, ali->hmmdesc);
    lip_write_int(f, ali->sqfrom);
    lip_write_int(f, ali->sqto);
    lip_write_int(f, ali->L);
}
