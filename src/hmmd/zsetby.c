#include "hmmd/zsetby.h"
#include "h3client/h3client.h"

enum h3c_rc hmmd_zsetby_unpack(enum hmmd_zsetby *dst,
                               unsigned char const **data)
{
    enum h3c_rc rc = H3C_OK;
    switch (**data)
    {
    case 0:
        *dst = p7_ZSETBY_NTARGETS;
        break;
    case 1:
        *dst = p7_ZSETBY_OPTION;
        break;
    case 2:
        *dst = p7_ZSETBY_FILEINFO;
        break;
    default:
        rc = H3C_FAILED_UNPACK;
        break;
    }
    (*data)++;
    return rc;
}
