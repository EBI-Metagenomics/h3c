#ifndef DIALER_H
#define DIALER_H

struct h3c_dialer;
struct h3c_stream;

struct h3c_dialer *h3c__dialer_new(char const *ip, int port);
int h3c__dialer_dial(struct h3c_dialer *, long deadline);
struct h3c_stream *h3c__dialer_stream(struct h3c_dialer *);
void h3c__dialer_del(struct h3c_dialer *);

#endif
