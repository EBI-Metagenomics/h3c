#include "hmmd/status.h"
#include "c_toolbelt/c_toolbelt.h"
#include <stdlib.h>
#include <string.h>

void hmmd_status_unpack(struct hmmd_status *status, unsigned char const *data)
{
    uint32_t n32 = 0;
    memcpy(&n32, data, sizeof(n32));

    status->status = ctb_ntohl(n32);

    data += sizeof(n32);

    uint64_t n64 = 0;
    memcpy(&n64, data, sizeof(n64));
    status->msg_size = ctb_ntohll(n64);
}
