#ifndef H3CLIENT_RC_H
#define H3CLIENT_RC_H

enum h3c_rc
{
    H3C_OK,
    H3C_NOT_ENOUGH_MEMORY,
    H3C_ARGS_TOO_LONG,
    H3C_INVALID_ADDRESS,
    H3C_FAILED_PARSE,
    H3C_FAILED_CLOSE,
    H3C_FAILED_CREATE_SOCKET,
    H3C_FAILED_CONNECT,
    H3C_FAILED_READ_FILE,
    H3C_FAILED_WRITE_SOCKET,
    H3C_FAILED_READ_SOCKET,
    H3C_FAILED_UNPACK,
    H3C_FAILED_PACK,
    H3C_INTEGER_OVERFLOW,
};

#endif
