#include "request.h"
#include "buff.h"
#include "c_toolbelt/c_toolbelt.h"
#include "h3client/h3client.h"
#include <stdlib.h>
#include <string.h>

#define BUFF_SIZE 2048

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

enum h3c_rc request_set_args(struct request *req, char const *args)
{
    char *data = (char *)req->buff->data;
    data[0] = '@';
    size_t size = ctb_strlcpy(data + 1, args, BUFF_SIZE - 2);
    if (size >= BUFF_SIZE - 2) return H3C_ARGS_TOO_LONG;
    data[size + 1] = '\n';
    data[size + 2] = '\0';
    return H3C_OK;
}

#define READ_SIZE 4096

enum h3c_rc request_set_seqs(struct request *req, FILE *fasta)
{
    enum h3c_rc rc = H3C_OK;
    size_t size = strlen((char const *)req->buff->data);
    if ((rc = buff_ensure(&req->buff, size + READ_SIZE))) goto cleanup;

    while (fgets((char *)req->buff->data + size, READ_SIZE, fasta))
    {
        size += strlen((char const *)req->buff->data + size);
        if ((rc = buff_ensure(&req->buff, size + READ_SIZE))) goto cleanup;
    }

    if ((rc = buff_ensure(&req->buff, size + 2))) goto cleanup;
    strcat((char *)req->buff->data + size, "//");

    if (!feof(fasta)) rc = H3C_FAILED_READ_FILE;

cleanup:
    return rc;
}

size_t request_size(struct request const *req)
{
    return strlen((char const *)req->buff->data);
}

char const *request_data(struct request const *req)
{
    return (char const *)req->buff->data;
}
