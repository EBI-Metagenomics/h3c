#include "h3client/request.h"
#include "c_toolbelt/c_toolbelt.h"
#include "h3client/buff.h"
#include <string.h>

#define BUFF_SIZE 2048

bool h3request_init(struct h3request *req)
{
    req->buff = buff_new(2048);
    return req->buff;
}

void h3request_setup_args(struct h3request *req, char const *args)
{
    char *data = (char *)req->buff->data;
    data[0] = '@';
    size_t size = ctb_strlcpy(data + 1, args, BUFF_SIZE - 2);
    data[size + 1] = '\n';
    data[size + 2] = '\0';
}

#define READ_SIZE 4096

bool h3request_setup_fasta(struct h3request *req, FILE *fasta)
{
    size_t size = strlen((char *)req->buff->data);
    if (!buff_ensure(&req->buff, size + READ_SIZE)) return false;

    while (fgets((char *)req->buff->data + size, READ_SIZE, fasta))
    {
        size += strlen((char *)req->buff->data + size);
        if (!buff_ensure(&req->buff, size + READ_SIZE)) return false;
    }

    if (!buff_ensure(&req->buff, size + 2)) return false;
    strcat((char *)req->buff->data + size, "//");

    return !!feof(fasta);
}

size_t h3request_size(struct h3request const *req)
{
    return strlen((char *)req->buff->data);
}

char const *h3request_data(struct h3request const *req)
{
    return (char const *)req->buff->data;
}

void h3request_cleanup(struct h3request *req) { buff_del(req->buff); }
