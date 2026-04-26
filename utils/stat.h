#include <stdlib.h>
#include "string_ops.h"
#include <linux/limits.h>
#include <sys/stat.h>

typedef struct
{
    bool exists;
    ssize_t file_size;
} fs_metadata;

static inline fs_metadata fs_get_metadata(string_view filename)
{
    char buf[PATH_MAX];
    struct stat st;
    fs_metadata metadata;
    metadata.exists = false;

    ssize_t len = filename.end - filename.start;
    if (len <= 0 || len + 1 >= PATH_MAX)
    {
        return metadata;
    }
    memcpy(buf, filename.start, filename.end - filename.start);
    buf[len] = '\0';

    if (stat(buf, &st) < 0)
    {
        return metadata;
    }

    metadata.exists = true;
    metadata.file_size = st.st_size;
    return metadata;
}