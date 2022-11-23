#ifndef MSG_H
#define MSG_H

#include "cco.h"

enum state
{
    SEND,
    RECV,
};

struct msg
{
    struct nng_stream *stream;
    enum state state;
    long deadline;
    struct nng_aio *aio;
    struct answer *ans;
    struct amsg *send_amsg;
    struct hmsg *recv_hmsg;
    struct nng_mtx *mtx;
    struct cco_node node;
};

struct h3c_result;

struct msg *h3c_msg_new(struct nng_stream *);
int h3c_msg_start(struct msg *, char const *args, char const *seq,
                  long deadline);
void h3c_msg_wait(struct msg *);
int h3c_msg_result(struct msg *);
void h3c_msg_cancel(struct msg *);
void h3c_msg_stop(struct msg *);
void h3c_msg_del(struct msg *);
struct answer *h3c_msg_answer(struct msg *);

#endif
