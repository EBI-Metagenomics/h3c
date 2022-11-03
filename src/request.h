#ifndef H3C_REQUEST_H
#define H3C_REQUEST_H

#include <stddef.h>
#include <stdio.h>

struct request;

struct request *request_new(void);
void request_del(struct request const *req);

enum h3c_rc request_set_args(struct request *req, char const *args);
enum h3c_rc request_set_seqs(struct request *req, FILE *fasta);
size_t request_size(struct request const *req);
char const *request_data(struct request const *req);

#endif
