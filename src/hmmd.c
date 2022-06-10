#include "h3client/hmmd.h"
#include "c_toolbelt/c_toolbelt.h"
#include <string.h>

void hmmd_status_unpack(unsigned char const *data, struct hmmd_status *status)
{
    uint32_t network_32bit = 0;
    memcpy(&network_32bit, data, sizeof(network_32bit));

    status->status = ctb_ntohl(network_32bit);

    data += sizeof(network_32bit);

    uint64_t network_64bit = 0;
    memcpy(&network_64bit, data, sizeof(network_64bit));
    status->msg_size = ctb_ntohll(network_64bit);
}
