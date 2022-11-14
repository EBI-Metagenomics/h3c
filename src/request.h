#ifndef H3C_REQUEST_H
#define H3C_REQUEST_H

struct request;

struct request *request_new(void);
void request_del(struct request const *req);

#endif
