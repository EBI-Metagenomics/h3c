#include "addr.h"

enum h3c_rc addr_setup(struct addr *addr, enum addr_ipv ipv, char const *ip,
                       uint16_t port)
{
    addr->addr.sin_family = ipv == IPV4 ? AF_INET : AF_INET6;
    addr->addr.sin_port = htons(port);
    if ((inet_pton(addr->addr.sin_family, ip, &addr->addr.sin_addr)) != 1)
        return H3C_INVALID_ADDRESS;
    return H3C_OK;
}

sa_family_t addr_family(struct addr const *addr)
{
    return addr->addr.sin_family;
}
