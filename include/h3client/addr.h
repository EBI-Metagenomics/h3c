#ifndef H3CLIENT_ADDR_H
#define H3CLIENT_ADDR_H

#include <stdlib.h>

#include <sys/socket.h>
#ifdef HAVE_NETINET_IN_H
/* On FreeBSD, you need netinet/in.h for struct sockaddr_in */
#include <netinet/in.h>
#endif
/* On OpenBSD, netinet/in.h is required for (must precede) arpa/inet.h  */
#include <arpa/inet.h>

enum h3addr_ipv
{
    IPV4,
    IPV6
};

struct h3addr
{
    struct sockaddr_in addr;
};

void h3addr_setup(struct h3addr *, enum h3addr_ipv, char const *ip,
                  uint16_t port);

sa_family_t h3addr_family(struct h3addr const *h3addr);

#endif
