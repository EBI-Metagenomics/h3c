#ifndef HMMD_ZSETBY_H
#define HMMD_ZSETBY_H

#include <stddef.h>

enum hmmd_zsetby
{
    HMMD_ZSETBY_NTARGETS = 0,
    HMMD_ZSETBY_OPTION = 1,
    HMMD_ZSETBY_FILEINFO = 2
};

enum h3c_rc hmmd_zsetby_parse(enum hmmd_zsetby *dst,
                              unsigned char const **data);

#endif
