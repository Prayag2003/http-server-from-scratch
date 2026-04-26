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

#endif /* STRING_OPS_H */
