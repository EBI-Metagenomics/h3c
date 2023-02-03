#include "h3c/fini.h"
#include "nng/nng.h"

void h3c_fini(void) { nng_fini(); }
