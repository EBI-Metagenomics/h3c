#ifndef DIALER_H
#define DIALER_H

struct dialer;
struct nng_stream;

struct dialer *dialer_new(char const *uri);
int dialer_open(struct dialer *, long deadline);
struct nng_stream *dialer_output(struct dialer *);
void dialer_close(struct dialer *);
void dialer_del(struct dialer *);

#endif
