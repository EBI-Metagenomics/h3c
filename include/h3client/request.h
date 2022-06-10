#ifndef H3CLIENT_REQUEST_H
#define H3CLIENT_REQUEST_H

#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>

struct buff;

struct h3request
{
    struct buff *buff;
};

bool h3request_init(struct h3request *req);
void h3request_setup_args(struct h3request *req, char const *args);
bool h3request_setup_fasta(struct h3request *req, FILE *fasta);
size_t h3request_size(struct h3request const *req);
char const *h3request_data(struct h3request const *req);
void h3request_cleanup(struct h3request *req);

#endif
