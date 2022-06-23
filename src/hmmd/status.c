#include "hmmd/status.h"
#include "c_toolbelt/c_toolbelt.h"
#include "h3client/rc.h"
#include "hmmd/utils.h"
#include "lite_pack/file/file.h"
#include "lite_pack/lite_pack.h"
#include <stdlib.h>
#include <string.h>

void hmmd_status_init(struct hmmd_status *st) { memset(st, 0, sizeof(*st)); }

void hmmd_status_cleanup(struct hmmd_status *st) { hmmd_status_init(st); }

void hmmd_status_deserialize(struct hmmd_status *status, size_t *read_size,
                        unsigned char const *data)
{
    unsigned char const *ptr = data;
    status->status = eatu32(&ptr);
    status->msg_size = eatu64(&ptr);
    *read_size = (size_t)(ptr - data);
}

enum h3c_rc hmmd_status_pack(struct hmmd_status const *st, struct lip_file *f)
{
    lip_write_int(f, st->status);
    lip_write_int(f, st->msg_size);

    return f->error ? H3C_FAILED_PACK : H3C_OK;
}
