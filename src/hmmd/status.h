#ifndef HMMD_STATUS_H
#define HMMD_STATUS_H

#include <stdint.h>

struct hmmd_status
{
    uint32_t status;
    uint64_t msg_size;
};

void hmmd_status_unpack(struct hmmd_status *status, unsigned char const *data);

#endif
