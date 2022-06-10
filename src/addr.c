#include "h3client/addr.h"

void h3addr_setup(struct h3addr *h3addr, enum h3addr_ipv ipv, char const *ip,
                  uint16_t port)
{
    h3addr->addr.sin_family = ipv == IPV4 ? AF_INET : AF_INET6;
    h3addr->addr.sin_port = htons(port);
    if ((inet_pton(h3addr->addr.sin_family, ip, &h3addr->addr.sin_addr)) != 1)
        exit(1);
}

sa_family_t h3addr_family(struct h3addr const *h3addr)
{
    return h3addr->addr.sin_family;
}
