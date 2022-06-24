#include "xxhash/xxhash.h"
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

#define BUFFSIZE (8 * 1024)

bool file_hash(FILE *restrict fp, int64_t *hash)
{
    bool ok = true;
    XXH3_state_t *state = XXH3_createState();
    if (!state)
    {
        ok = false;
        goto cleanup;
    }
    XXH3_64bits_reset(state);

    size_t n = 0;
    unsigned char buffer[BUFFSIZE] = {0};
    while ((n = fread(buffer, sizeof(*buffer), BUFFSIZE, fp)) > 0)
    {
        if (n < BUFFSIZE && ferror(fp))
        {
            ok = false;
            goto cleanup;
        }

        XXH3_64bits_update(state, buffer, n);
    }
    if (ferror(fp))
    {
        ok = false;
        goto cleanup;
    }

    union
    {
        int64_t const i;
        uint64_t const u;
    } const h = {.u = XXH3_64bits_digest(state)};
    *hash = h.i;

cleanup:
    XXH3_freeState(state);
    return ok;
}
