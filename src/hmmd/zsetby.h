#ifndef HMMD_ZSETBY_H
#define HMMD_ZSETBY_H

enum hmmd_zsetby
{
    p7_ZSETBY_NTARGETS = 0,
    p7_ZSETBY_OPTION = 1,
    p7_ZSETBY_FILEINFO = 2
};

enum h3c_rc hmmd_zsetby_unpack(enum hmmd_zsetby *dst,
                               unsigned char const **data);

#endif
