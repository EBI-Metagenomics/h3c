#include "h3client/h3client.h"

#define _POSIX_C_SOURCE 200112L
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

static char const *h3master_address(void)
{
    char const *addr = getenv("H3CLIENT_MASTER_ADDRESS");
    return addr ? addr : "127.0.0.1";
}

int main(void)
{
    if (h3c_open(h3master_address(), 51371))
    {
        printf("Failed to h3c_open!");
        return 1;
    }

    FILE *file = fopen(ASSETS "/ross.fasta", "r");
    if (!file)
    {
        printf("Failed to fopen!");
        return 1;
    }

    if (h3c_call("--hmmdb 1 --acc --cut_ga", file))
    {
        printf("Failed to h3c_call!");
        return 1;
    }
    fclose(file);

    if (h3c_close())
    {
        printf("Failed to h3c_close!");
        return 1;
    }

    return 0;
}
