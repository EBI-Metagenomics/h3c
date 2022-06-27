#include "strxdup.h"
#include "c_toolbelt/c_toolbelt.h"
#include <assert.h>
#include <string.h>

char *strxdup(char *dst, char const *src)
{
    assert(dst != 0);
    if (!src)
    {
        dst[0] = 0;
        return dst;
    }

    size_t sz = strlen(src);
    void *t = ctb_realloc(dst, sz + 1);
    return t ? memcpy(t, src, sz + 1) : 0;
}
