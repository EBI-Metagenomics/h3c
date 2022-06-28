#include "hmmd/alidisplay.h"
#include "c_toolbelt/c_toolbelt.h"
#include "h3client/h3client.h"
#include "hmmd/utils.h"
#include <assert.h>
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

enum h3c_rc hmmd_alidisplay_parse(struct hmmd_alidisplay *ali,
                                  size_t *read_size, unsigned char const *data)
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
        rc = H3C_FAILED_PARSE;
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
        rc = H3C_FAILED_PARSE;
        goto cleanup;
    }

    *read_size = (size_t)(ptr - data);
    return H3C_OK;

cleanup:
    hmmd_alidisplay_cleanup(ali);
    return rc;
}
