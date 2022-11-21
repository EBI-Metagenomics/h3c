#ifndef H3C_DIALER_H
#define H3C_DIALER_H

struct h3c_dialer;
struct h3c_stream;

struct h3c_dialer *h3c_dialer_new(char const *ip, int port);
int h3c_dialer_dial(struct h3c_dialer *, long deadline);
struct h3c_stream *h3c_dialer_stream(struct h3c_dialer *x);
void h3c_dialer_del(struct h3c_dialer *);

#endif
