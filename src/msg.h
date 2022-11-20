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

struct msg *msg_new(struct nng_stream *);
int msg_start(struct msg *, char const *args, char const *seq, long deadline);
int msg_wait(struct msg *);
void msg_cancel(struct msg *);
void msg_del(struct msg *);
struct answer *msg_answer(struct msg *);

#endif
