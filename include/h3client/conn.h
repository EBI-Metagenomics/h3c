#ifndef H3CLIENT_CONN_H
#define H3CLIENT_CONN_H

#include <stdbool.h>
#include <stdint.h>

struct h3request;
struct h3answer;

bool h3conn_open(char const *ip, uint16_t port);
bool h3conn_call(struct h3request const *req, struct h3answer *ans);
void h3conn_close(void);

#endif
