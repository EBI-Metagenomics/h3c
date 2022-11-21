#ifndef STREAM_H
#define STREAM_H

struct h3c_stream;
struct nng_stream;

struct h3c_stream *h3c_stream_new(struct nng_stream *stream);

#endif
