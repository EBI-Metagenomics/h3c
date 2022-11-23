#include "hmmd/zsetby.h"
#include "h3c/h3c.h"

int h3c_hmmd_zsetby_parse(enum hmmd_zsetby *dst, unsigned char const **data)
{
    int rc = H3C_OK;
    switch (**data)
    {
    case 0:
        *dst = HMMD_ZSETBY_NTARGETS;
        break;
    case 1:
        *dst = HMMD_ZSETBY_OPTION;
        break;
    case 2:
        *dst = HMMD_ZSETBY_FILEINFO;
        break;
    default:
        rc = H3C_EUNPACK;
        break;
    }
    (*data) += 1;
    return rc;
}
