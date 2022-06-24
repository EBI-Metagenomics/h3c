#include "h3client/h3client.h"
#include "file_hash.h"

#ifdef _POSIX_C_SOURCE
#if _POSIX_C_SOURCE < 200112L
#undef _POSIX_C_SOURCE
#define _POSIX_C_SOURCE 200112L
#endif
#else
#define _POSIX_C_SOURCE 200112L
#endif

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
        printf("Failed to h3c_open!\n");
        return 1;
    }

    FILE *file = fopen(ASSETS "/ross.fasta", "r");
    if (!file)
    {
        printf("Failed to fopen!\n");
        return 1;
    }

    if (h3c_call("--hmmdb 1 --acc --cut_ga", file))
    {
        printf("Failed to h3c_call!\n");
        return 1;
    }
    fclose(file);

    file = fopen(TMPDIR "/h3result.msgpack", "wb");
    if (!file)
    {
        printf("Failed to fopen!\n");
        return 1;
    }
    if (h3c_pack_answer(file))
    {
        printf("Failed to h3c_pack_answer!\n");
        return 1;
    }
    fclose(file);

    file = fopen(TMPDIR "/h3result.msgpack", "rb");
    if (!file)
    {
        printf("Failed to fopen!\n");
        return 1;
    }
    int64_t hash = 0;
    if (!file_hash(file, &hash))
    {
        printf("Failed to file_hash!\n");
        return 1;
    }
    // printf("\nhash: %lld\n", hash);
    if (hash != -2523193986434044269LL)
    {
        printf("Wrong file hash for h3result.msgpack!\n");
        return 1;
    }
    fclose(file);

    h3c_pack_print();

    if (h3c_close())
    {
        printf("Failed to h3c_close!\n");
        return 1;
    }

    return 0;
}
