#include "h3c/now.h"
#include <nng/nng.h>
#include <nng/supplemental/util/platform.h>

long h3c_now(void) { return (long)nng_clock(); }
