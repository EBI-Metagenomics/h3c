#include "hmmd/status.h"
#include "h3c/errno.h"
#include "utils.h"
#include <stdlib.h>
#include <string.h>

void h3c_hmmd_status_init(struct hmmd_status *st)
{
    memset(st, 0, sizeof(*st));
}

void h3c_hmmd_status_parse(struct hmmd_status *status, size_t *read_size,
                           unsigned char const *data)
{
    unsigned char const *ptr = data;
    status->status = h3c_eatu32(&ptr);
    status->msg_size = h3c_eatu64(&ptr);
    *read_size = (size_t)(ptr - data);
}
