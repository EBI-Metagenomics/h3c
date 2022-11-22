#include "h3c/deadline.h"
#include <nng/nng.h>
#include <nng/supplemental/util/platform.h>

long h3c_deadline(long timeout) { return (long)nng_clock() + timeout; }
