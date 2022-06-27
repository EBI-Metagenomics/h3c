#ifndef DEL_H
#define DEL_H

#include <stdlib.h>

#define DEL(ptr)                                                               \
    do                                                                         \
    {                                                                          \
        free((ptr));                                                           \
        (ptr) = 0;                                                             \
    } while (0);

#endif
