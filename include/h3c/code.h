#ifndef H3C_CODE_H
#define H3C_CODE_H

#include "h3c/export.h"

enum
{
    H3C_OK,
    H3C_END,
    H3C_EUNKNOWN,
    H3C_EPARSE,
    H3C_EUNPACK,
    H3C_EPACK,
    H3C_EUNEOF,
    H3C_EOUTRANGE,
    H3C_ETIMEDOUT,
    H3C_ECANCELED,
    H3C_EADDRINVAL,
    H3C_ECLOSED,
    H3C_ECONNREFUSED,
    H3C_ECONNRESET,
    H3C_EINVAL,
    H3C_ENOMEM,
    H3C_EPEERAUTH,
    H3C_EPROTO,
    H3C_EUNREACHABLE,
    H3C_ECONNSHUT,
};

H3C_API char const *h3c_decode(int code);

#endif
