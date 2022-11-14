#include "request.h"
#include "buff.h"
#include "h3c/rc.h"
#include <stdlib.h>

struct request
{
    struct buff *buff;
};

struct request *request_new(void)
{
    struct request *req = malloc(sizeof(*req));
    if (!req) return 0;
    if (!(req->buff = buff_new(2048)))
    {
        free(req);
        return 0;
    }
    return req;
}

void request_del(struct request const *req)
{
    buff_del(req->buff);
    free((void *)req);
}
