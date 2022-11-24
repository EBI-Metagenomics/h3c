#include "fs.h"
#include "h3c/h3c.h"
#include "helper.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define PORT 51379
static char const cmd[] = "--hmmdb 1 --acc --cut_ga";

static void test_multi(void);

int main(void)
{
    atexit(h3c_fini);
    test_multi();
    return EXIT_SUCCESS;
}

static void test_multi(void)
{
    struct h3c_dialer *d = h3c_dialer_new("127.0.0.1", PORT);
    if (!d) fail();
    check_code(h3c_dialer_dial(d, h3c_deadline(1000)));

    struct h3c_stream *s = h3c_dialer_stream(d);

    long deadline = h3c_deadline(1000 * 5);
    for (size_t i = 0; i < array_size(ross); ++i)
        check_code(h3c_stream_put(s, cmd, ross[i].name, ross[i].seq, deadline));

    h3c_stream_del(s);
    h3c_dialer_del(d);
}
