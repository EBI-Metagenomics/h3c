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

enum addr_ipv
{
    IPV4,
    IPV6
};

struct addr
{
    struct sockaddr_in addr;
};

enum h3c_rc addr_setup(struct addr *, enum addr_ipv, char const *ip,
                       uint16_t port);

sa_family_t addr_family(struct addr const *addr);

#endif
