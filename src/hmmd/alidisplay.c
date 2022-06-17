#include "hmmd/alidisplay.h"
#include "c_toolbelt/c_toolbelt.h"
#include "h3client/h3client.h"
#include "hmmd/utils.h"
#include <stdlib.h>
#include <string.h>

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

    size_t obj_size = eat32(&ptr);

    if (!(ali->mem = ctb_realloc(ali->mem, obj_size - SER_BASE_SIZE)))
    {
        rc = H3C_NOT_ENOUGH_MEMORY;
        goto cleanup;
    }
    ali->memsize = obj_size - SER_BASE_SIZE;

    ali->N = eat32(&ptr);
    ali->hmmfrom = eat32(&ptr);
    ali->hmmto = eat32(&ptr);
    ali->M = eat32(&ptr);
    ali->sqfrom = eat64(&ptr);
    ali->sqto = eat64(&ptr);
    ali->L = eat64(&ptr);

    uint8_t presence = eat8(&ptr);

    memcpy(ali->mem, ptr, obj_size - SER_BASE_SIZE);
    ptr += (obj_size - SER_BASE_SIZE);

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

    return H3C_OK;

cleanup:
    return rc;
}
