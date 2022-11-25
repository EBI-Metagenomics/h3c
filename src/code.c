#include "h3c/code.h"

char const *h3c_decode(int c)
{
    if (c == H3C_OK) return "success";
    if (c == H3C_END) return "end of tasks";
    if (c == H3C_EUNKNOWN) return "unknown error";
    if (c == H3C_EPARSE) return "parse error";
    if (c == H3C_EUNPACK) return "unpack failure";
    if (c == H3C_EPACK) return "pack failure";
    if (c == H3C_EUNEOF) return "unexpected end-of-file";
    if (c == H3C_EOUTRANGE) return "out-of-range integer";
    if (c == H3C_ETIMEDOUT) return "timed out";
    if (c == H3C_ECANCELED) return "operation was canceled";
    if (c == H3C_EADDRINVAL) return "invalid address";
    if (c == H3C_ECLOSED) return "operation was aborted";
    if (c == H3C_ECONNREFUSED) return "connection was refused";
    if (c == H3C_ECONNRESET) return "connection was reset";
    if (c == H3C_EINVAL) return "invalid url";
    if (c == H3C_ENOMEM) return "insufficient free memory";
    if (c == H3C_EPEERAUTH) return "authentication failure";
    if (c == H3C_EPROTO) return "protocol error occurred";
    if (c == H3C_EUNREACHABLE) return "address is not reachable";
    if (c == H3C_ECONNSHUT) return "remote peer shutdown after sending data";
    return "(null)";
}
