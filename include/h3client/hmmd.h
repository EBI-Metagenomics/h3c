#ifndef H3CLIENT_HMMD_H
#define H3CLIENT_HMMD_H

#include <stdint.h>

#define HMMD_STATUS_PACK_SIZE (sizeof(uint32_t) + sizeof(uint64_t))

struct hmmd_status
{
    uint32_t status;
    uint64_t msg_size;
};

void hmmd_status_unpack(unsigned char const *data, struct hmmd_status *status);

#endif
