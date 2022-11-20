#ifndef DIALER_H
#define DIALER_H

struct dialer;
struct sock;

struct dialer *dialer_new(char const *uri);
int dialer_dial(struct dialer *, long deadline);
struct sock *dialer_sock(struct dialer *);
void dialer_del(struct dialer *);

#endif
