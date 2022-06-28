#ifndef RESULT_H
#define RESULT_H

#include "stats.h"
#include "tophits.h"

struct h3c_result
{
    struct stats stats;
    struct tophits tophits;
};

#endif
