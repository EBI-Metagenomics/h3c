#include "utils.h"
#include "c_toolbelt/c_toolbelt.h"
#include "h3client/rc.h"
#include "lite_pack/lite_pack.h"
#include <string.h>

bool expect_key(struct lip_file *f, char const *key)
{
    char str[16] = {0};
    unsigned size = (unsigned)strlen(key) + 1;
    lip_read_cstr(f, size, str);

    return !lip_file_error(f) && strncmp(str, key, sizeof(str)) == 0;
}

bool expect_array_size(struct lip_file *f, unsigned size)
{
    unsigned sz = 0;
    lip_read_array_size(f, &sz);
    return !lip_file_error(f) && size == sz;
}

bool expect_map_size(struct lip_file *f, unsigned size)
{
    unsigned sz = 0;
    lip_read_map_size(f, &sz);
    return !lip_file_error(f) && size == sz;
}

enum h3c_rc read_string(struct lip_file *f, char **str)
{
    unsigned size = 0;
    if (!lip_read_str_size(f, &size)) return H3C_FAILED_UNPACK;
    if (!(*str = ctb_realloc(*str, size + 1))) return H3C_NOT_ENOUGH_MEMORY;
    if (!lip_read_str_data(f, size, *str)) return H3C_FAILED_UNPACK;
    (*str)[size] = 0;
    return H3C_OK;
}
