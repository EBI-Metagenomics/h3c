#include <signal.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include <sys/socket.h>
#ifdef HAVE_NETINET_IN_H
/* On FreeBSD, you need netinet/in.h for struct sockaddr_in */
#include <netinet/in.h>
#endif
/* On OpenBSD, netinet/in.h is required for (must precede) arpa/inet.h  */
#include <arpa/inet.h>
#include <errno.h>
#include <unistd.h>

#include "h3client/addr.h"
#include "h3client/answer.h"
#include "h3client/conn.h"
#include "h3client/request.h"

struct h3conn
{
    struct h3addr addr;
    int sockfd;

} conn = {0};

bool h3conn_open(char const *ip, uint16_t port)
{
    h3addr_setup(&conn.addr, IPV4, ip, port);

    if ((conn.sockfd = socket(h3addr_family(&conn.addr), SOCK_STREAM, 0)) == -1)
        return false;

    if (connect(conn.sockfd, (struct sockaddr *)&conn.addr,
                sizeof(conn.addr)) == -1)
        return false;

    return true;
}

static bool writen(int fd, void const *buf, size_t count)
{
    while (count > 0)
    {
        ssize_t n = write(fd, buf, count);
        if (n == -1)
        {
            if (errno == EINTR)
            {
                n = 0;
                errno = 0;
            }
            else
            {
                errno = 0;
                return false;
            }
        }
        count -= n;
        buf += n;
    }

    return true;
}

static bool readn(int fd, void *buf, size_t count)
{
    while (count > 0)
    {
        ssize_t n = read(fd, buf, count);
        if (n == -1)
        {
            if (errno == EINTR)
            {
                n = 0;
                errno = 0;
            }
            else
            {
                errno = 0;
                return false;
            }
        }

        count -= n;
        buf += n;
    }

    return count == 0;
}

bool h3conn_call(struct h3request const *req, struct h3answer *ans)
{
    size_t size = h3request_size(req);
    if (!writen(conn.sockfd, h3request_data(req), size)) return false;

    void *data = h3answer_status_data(ans);
    size = h3answer_status_size();
    if (!readn(conn.sockfd, data, size)) return false;

    struct hmmd_status const *status = h3answer_status_unpack(ans);

    size = status->msg_size;
    if (!h3answer_ensure(ans, size)) return false;

    data = h3answer_data(ans);
    if (!readn(conn.sockfd, data, size)) return false;

    // if (!status->status)
    // {
    //     if (!h3answer_unpack(ans)) return false;
    // }

    return true;
}

void h3conn_close(void) { close(conn.sockfd); }
