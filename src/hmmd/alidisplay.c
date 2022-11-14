#include "hmmd/alidisplay.h"
#include "h3c/h3c.h"
#include "utils.h"
#include "zc.h"
#include <assert.h>
#include <stdlib.h>
#include <string.h>

void hmmd_alidisplay_init(struct hmmd_alidisplay *ali)
{
    memset(ali, 0, sizeof(*ali));
}

void hmmd_alidisplay_cleanup(struct hmmd_alidisplay *ali)
{
    if (ali->mem) DEL(ali->mem);
    hmmd_alidisplay_init(ali);
}

static_assert(sizeof(int) == sizeof(uint32_t), "HMMER3 undefired requirement");
#define SER_BASE_SIZE ((5 * sizeof(int)) + (3 * sizeof(int64_t)) + 1)

#define RFLINE_PRESENT (1 << 0)
#define MMLINE_PRESENT (1 << 1)
#define CSLINE_PRESENT (1 << 2)
#define PPLINE_PRESENT (1 << 3)
#define ASEQ_PRESENT (1 << 4)
#define NTSEQ_PRESENT (1 << 5)

static bool parse_strings(struct hmmd_alidisplay *ali, size_t size, char **mem)
{
    unsigned nstrings = 0;
    nstrings += !!(ali->presence & RFLINE_PRESENT);
    nstrings += !!(ali->presence & MMLINE_PRESENT);
    nstrings += !!(ali->presence & CSLINE_PRESENT);
    nstrings += 2;
    nstrings += !!(ali->presence & ASEQ_PRESENT);
    nstrings += !!(ali->presence & NTSEQ_PRESENT);
    nstrings += !!(ali->presence & PPLINE_PRESENT);
    nstrings += 6;

    if (!expect_n_strings(size, *mem, nstrings)) return false;

    ali->rfline = ali->presence & RFLINE_PRESENT ? strskip(mem) : 0;
    ali->mmline = ali->presence & MMLINE_PRESENT ? strskip(mem) : 0;
    ali->csline = ali->presence & CSLINE_PRESENT ? strskip(mem) : 0;
    ali->model = strskip(mem);
    ali->mline = strskip(mem);
    ali->aseq = ali->presence & ASEQ_PRESENT ? strskip(mem) : 0;
    ali->ntseq = ali->presence & NTSEQ_PRESENT ? strskip(mem) : 0;
    ali->ppline = ali->presence & PPLINE_PRESENT ? strskip(mem) : 0;

    ali->hmmname = strskip(mem);
    ali->hmmacc = strskip(mem);
    ali->hmmdesc = strskip(mem);
    ali->sqname = strskip(mem);
    ali->sqacc = strskip(mem);
    ali->sqdesc = strskip(mem);

    return true;
}

enum h3c_rc hmmd_alidisplay_parse(struct hmmd_alidisplay *ali,
                                  unsigned char const **ptr,
                                  unsigned char const *end)
{
    enum h3c_rc rc = H3C_OK;

    ESCAPE_OVERRUN(rc, *ptr, end, sizeof(uint32_t));

    size_t obj_size = eatu32(ptr);
    if (obj_size <= SER_BASE_SIZE)
    {
        rc = H3C_FAILED_PARSE;
        goto cleanup;
    }
    size_t memsize = (size_t)(obj_size - SER_BASE_SIZE);

    if (!(ali->mem = zc_reallocf(ali->mem, memsize)))
    {
        rc = H3C_NOMEM;
        goto cleanup;
    }
    ali->memsize = memsize;

    ESCAPE_OVERRUN(rc, *ptr, end, 4 * sizeof(uint32_t) + 3 * sizeof(uint64_t));
    ali->N = eatu32(ptr);
    ali->hmmfrom = eatu32(ptr);
    ali->hmmto = eatu32(ptr);
    ali->M = eatu32(ptr);
    ali->sqfrom = eatu64(ptr);
    ali->sqto = eatu64(ptr);
    ali->L = eatu64(ptr);

    ESCAPE_OVERRUN(rc, *ptr, end, sizeof(uint8_t));
    ali->presence = eatu8(ptr);

    ESCAPE_OVERRUN(rc, *ptr, end, memsize);
    memcpy(ali->mem, *ptr, memsize);
    *ptr += memsize;

    char *mem = ali->mem;
    if (!parse_strings(ali, ali->memsize, &mem))
    {
        rc = H3C_FAILED_PARSE;
        goto cleanup;
    }

    return H3C_OK;

cleanup:
    hmmd_alidisplay_cleanup(ali);
    return rc;
}
