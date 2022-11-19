#include "buff.h"
#include "h3c/code.h"
#include <assert.h>
#include <stdlib.h>

struct buff *buff_new(size_t capacity)
{
    assert(capacity > 0);

    struct buff *buff = malloc(sizeof(struct buff) + capacity);
    if (!buff) return 0;

    buff->size = 0;
    buff->capacity = capacity;
    return buff;
}

int buff_ensure(struct buff **buff, size_t capacity)
{
    if (capacity > (*buff)->capacity)
    {
        struct buff *tmp = realloc(*buff, sizeof(*tmp) + capacity);
        if (!tmp) return H3C_ENOMEM;

        *buff = tmp;
        (*buff)->capacity = capacity;
    }
    return H3C_OK;
}

void buff_del(struct buff const *buff) { free((void *)buff); }
