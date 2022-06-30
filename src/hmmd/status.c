#include "hmmd/status.h"
#include "c_toolbelt/c_toolbelt.h"
#include "h3client/rc.h"
#include "utils.h"
#include <stdlib.h>
#include <string.h>

void hmmd_status_init(struct hmmd_status *st) { memset(st, 0, sizeof(*st)); }

void hmmd_status_cleanup(struct hmmd_status *st) { hmmd_status_init(st); }

void hmmd_status_parse(struct hmmd_status *status, size_t *read_size,
                       unsigned char const *data)
{
    unsigned char const *ptr = data;
    status->status = eatu32(&ptr);
    status->msg_size = eatu64(&ptr);
    *read_size = (size_t)(ptr - data);
}
