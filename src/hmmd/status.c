#include "hmmd/status.h"
#include "c_toolbelt/c_toolbelt.h"
#include "hmmd/utils.h"
#include <stdlib.h>
#include <string.h>

void hmmd_status_init(struct hmmd_status *st) { memset(st, 0, sizeof(*st)); }

void hmmd_status_cleanup(struct hmmd_status *st) { hmmd_status_init(st); }

void hmmd_status_unpack(struct hmmd_status *status, size_t *read_size,
                        unsigned char const *data)
{
    unsigned char const *ptr = data;
    status->status = eat32(&ptr);
    status->msg_size = eat64(&ptr);
    *read_size = (size_t)(ptr - data);
}
