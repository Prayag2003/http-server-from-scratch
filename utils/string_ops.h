#ifndef STRING_OPS_H
#define STRING_OPS_H

#include <stddef.h>
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

typedef struct
{
    const char *data;
    size_t len;
} string;

typedef struct
{
    const char *start;
    const char *end;
} string_view;

typedef struct
{
    string_view *splits;
    size_t count;
    size_t capacity;
} string_splits;

/**
 * Compare two string structures for equality
 */
static inline bool strings_equal(string a, string b)
{
    if (a.len != b.len)
    {
        return false;
    }
    return strncmp(a.data, b.data, a.len) == 0;
}

/**
 * Create a string from a null-terminated C string
 */
static inline string string_from_cstr(const char *cstr)
{
    string result;
    result.data = cstr;
    result.len = strlen(cstr);
    return result;
}

/**
 * Convert a string to a string_view
 */
static inline string_view string_to_view(string s)
{
    string_view view;
    view.start = s.data;
    view.end = s.data + s.len;
    return view;
}

/**
 * Split a buffer by a delimiter character
 */
static string_splits split_string(const char *buf, size_t len, char split_by)
{
    string_splits result;
    const char *start;
    size_t result_i = 0;

    result.capacity = 8;
    result.count = 0;
    result.splits = (string_view *)calloc(sizeof(string_view), result.capacity);

    start = buf; /* Initialize start to beginning of buffer */

    for (size_t i = 0; i < len; i++)
    {
        if (buf[i] == split_by)
        {
            result.splits[result_i].start = start;
            result.splits[result_i].end = &buf[i];
            result_i++;
            result.count++;
            start = &buf[i] + 1; /* Skip past the delimiter */

            if (result.count == result.capacity)
            {
                result.capacity *= 2;
                string_view *new_splits = (string_view *)realloc(result.splits, sizeof(string_view) * result.capacity);
                if (!new_splits)
                {
                    perror("Failed to reallocate memory for string splits\n");
                    abort();
                }
                result.splits = new_splits;
            }
        }
    }

    /* Capture the final token after the last delimiter */
    if (start < &buf[len])
    {
        result.splits[result_i].start = start;
        result.splits[result_i].end = &buf[len];
        result.count++;
    }

    return result;
}

/**
 * Free allocated memory for string splits
 */
static void free_splits(string_splits *splits)
{
    if (splits)
    {
        free(splits->splits);
        splits->splits = NULL;
    }
}

#endif /* STRING_OPS_H */
