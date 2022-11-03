#include "h3client/h3client.h"
#include <errno.h>
#include <signal.h>
#include <stdio.h>
#include <string.h>

#define READ_SIZE 4096

static char buf[READ_SIZE] = {0};
static struct h3c_result *result = NULL;
static char filename[FILENAME_MAX] = "";

enum state
{
    BEGIN_SEQ,
    SEND_SEQ,
    END_SEQ,
    DONE,
    EXIT,
};

static bool init(void);
static bool cleanup(void);
static bool begin_seq(enum state *next);
static bool send_seq(enum state *next);
static bool end_seq(enum state *next);
static bool done(enum state *next);

int main(void)
{
    if (!init()) return 1;

    enum state state = BEGIN_SEQ;
    while (state != EXIT)
    {
        if (state == BEGIN_SEQ && !begin_seq(&state)) goto exit_early;
        if (state == SEND_SEQ && !send_seq(&state)) goto exit_early;
        if (state == END_SEQ && !end_seq(&state)) goto exit_early;
        if (state == DONE && !done(&state)) goto exit_early;
    }

    return !cleanup();

exit_early:
    cleanup();
    return 1;
}

static bool init(void)
{
    int rc = H3C_OK;
    if ((rc = h3c_open("127.0.0.1", 51371)))
    {
        fprintf(stderr, "h3c_open\n");
        return false;
    }
    if (!(result = h3c_result_new()))
    {
        h3c_close();
        return false;
    }
    return true;
}

static bool begin(void)
{
    if (h3c_begin("--hmmdb 1 --acc --cut_ga"))
    {
        fprintf(stderr, "h3c_begin\n");
        return false;
    }
    return true;
}

static bool send(char const *data)
{
    if (h3c_send(data))
    {
        fprintf(stderr, "h3c_send\n");
        return false;
    }
    return true;
}

static bool end(void)
{
    if (h3c_end(result))
    {
        fprintf(stderr, "h3c_end\n");
        return false;
    }
    return true;
}

static bool cleanup(void)
{
    int rc = H3C_OK;
    if ((rc = h3c_close())) fprintf(stderr, "h3c_close\n");
    h3c_result_del(result);
    return rc ? false : true;
}

static void remove_newline(char const *str)
{
    char *p = strrchr(str, '\n');
    if (p) *p = '\0';
}

static bool begin_seq(enum state *next)
{
    if (fgets(buf, sizeof buf, stdin))
    {
        if (!strcmp(buf, "@exit\n"))
        {
            *next = EXIT;
            return true;
        }
        if (strncmp(buf, "@file ", 6)) return false;
        remove_newline(buf);
        strcpy(filename, buf + 6);
        if (!begin()) return false;
    }
    *next = SEND_SEQ;
    return !feof(stdin);
}

static bool send_seq(enum state *next)
{
    while (fgets(buf, sizeof buf, stdin))
    {
        if (!strcmp(buf, "@exit\n"))
        {
            *next = EXIT;
            return true;
        }
        if (!strcmp(buf, "//\n")) break;
        if (!send(buf)) return false;
    }
    *next = END_SEQ;
    return !feof(stdin);
}

static bool end_seq(enum state *next)
{
    if (!strcmp(buf, "@exit\n"))
    {
        *next = EXIT;
        return true;
    }
    *next = DONE;
    if (!end()) return false;

    FILE *file = fopen(filename, "wb");
    if (!file) return false;

    if (h3c_result_pack(result, file))
    {
        fclose(file);
        return false;
    }
    return fclose(file) ? false : true;
}

static bool done(enum state *next)
{
    *next = BEGIN_SEQ;
    return puts(filename) >= 0;
}
