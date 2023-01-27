#ifndef H3C_DIALER_H
#define H3C_DIALER_H

#include "h3c/export.h"

struct h3c_dialer;
struct h3c_stream;

H3C_API struct h3c_dialer *h3c_dialer_new(char const *ip, int port);
H3C_API int h3c_dialer_dial(struct h3c_dialer *, long deadline);
H3C_API struct h3c_stream *h3c_dialer_stream(struct h3c_dialer *x);
H3C_API void h3c_dialer_del(struct h3c_dialer *);

#endif
