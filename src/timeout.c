#include "timeout.h"
#include <nng/nng.h>
#include <nng/supplemental/util/platform.h>

int timeout(long deadline)
{
    int timeout = deadline - nng_clock();
    if (timeout < 0) timeout = 0;
    return timeout;
}