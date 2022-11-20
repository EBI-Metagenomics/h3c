#ifndef ATOMIC_H
#define ATOMIC_H

#include <stdatomic.h>

#define atomic_release(object, value)                                          \
    atomic_store_explicit((object), (value), memory_order_release)

#define atomic_consume(object)                                                 \
    atomic_load_explicit((object), memory_order_consume)

#define atomic_increment(object) atomic_fetch_add((object), 1);

#endif
