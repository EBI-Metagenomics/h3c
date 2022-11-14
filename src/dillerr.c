#include "h3c/rc.h"
#define _POSIX_SOURCE 200809L
#include <libdill.h>

int dillerr(int rc)
{
    if (rc == ETIMEDOUT) return H3C_TIMEOUT;
    if (rc == ECANCELED) return H3C_CANCELED;
    if (rc == ECONNRESET) return H3C_CONNRESET;
    if (rc == ECONNREFUSED) return H3C_CONNREFUSED;
    if (rc == EPIPE) return H3C_PIPE;
    if (rc == EADDRNOTAVAIL) return H3C_ADDRNOTAVAIL;
    return H3C_ERROR;
}
