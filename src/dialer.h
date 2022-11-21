#ifndef DIALER_H
#define DIALER_H

struct h3c_dialer;
struct h3c_stream;

struct h3c_dialer *dialer_new(char const *ip, int port);
int dialer_dial(struct h3c_dialer *, long deadline);
struct h3c_stream *dialer_stream(struct h3c_dialer *);
void dialer_del(struct h3c_dialer *);

#endif
