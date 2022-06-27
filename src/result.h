#ifndef RESULT_H
#define RESULT_H

#include "stats.h"
#include "tophits.h"

struct h3c_result
{
    struct h3c_stats stats;
    struct h3c_tophits tophits;
};

#endif
