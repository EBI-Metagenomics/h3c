#include "fs.h"
#include "h3c/h3c.h"
#include "helper.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define PORT 51379
#define ROSS_GOOD 0
#define ROSS_BAD 1
static char const *seqs[2] = {0};
static char const cmd[] = "--hmmdb 1 --acc --cut_ga";

static void test_multi(void);

int main(void)
{
    atexit(h3c_fini);
    test_multi();
    return EXIT_SUCCESS;
}

static void assets_setup(void);
static void assets_cleanup(void);

static void test_multi(void)
{
    assets_setup();
    struct h3c_dialer *d = h3c_dialer_new("127.0.0.1", PORT);
    if (!d) fail();
    check_code(h3c_dialer_dial(d, h3c_now() + 1000));

    struct h3c_stream *s = h3c_dialer_stream(d);

    long deadline = h3c_now() + 1000 * 5;
    for (size_t i = 0; i < array_size(seqs); ++i)
        check_code(h3c_stream_put(s, cmd, seqs[i], deadline));

    h3c_stream_del(s);
    h3c_dialer_del(d);
    assets_cleanup();
}

static void assets_setup(void)
{
    long size = 0;
    unsigned char *data = NULL;

    check_code(fs_readall(ASSETS "/ross.fasta", &size, &data));
    seqs[ROSS_GOOD] = append_char(size, (char *)data, '\0');

    check_code(fs_readall(ASSETS "/ross.poor.fasta", &size, &data));
    seqs[ROSS_BAD] = append_char(size, (char *)data, '\0');
}
static void assets_cleanup(void)
{
    for (size_t i = 0; i < array_size(seqs); ++i)
        free((void *)seqs[i]);
}
