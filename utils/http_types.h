#ifndef HTTP_TYPES_H
#define HTTP_TYPES_H

#include <string.h>
#include "string_ops.h"

typedef struct
{
    string method;
    string uri;
    string version;
} http_req_line;

typedef enum
{
    HTTP_RES_OK = 200,
    HTTP_RES_INTERNAL_SERVER_ERR = 500,
    HTTP_RES_BAD_REQUEST = 400,
} http_result;

/**
 * Initialize an HTTP request line structure
 */
http_req_line http_req_line_init()
{
    http_req_line line;
    memset(&line, 0, sizeof(line));
    return line;
}

/**
 * Parse HTTP request line from a buffer
 */
http_result parse_req_line(const char *buf, size_t len, http_req_line *req_line)
{
    if (!buf || !req_line)
    {
        return HTTP_RES_INTERNAL_SERVER_ERR;
    }

    string_splits splits = split_string(buf, len, ' ');
    if (splits.count != 3)
    {
        printf("Invalid request line, expected 3 parts but got %zu\n", splits.count);
        return HTTP_RES_BAD_REQUEST;
    }
    req_line->method.data = splits.splits[0].start;
    req_line->method.len = splits.splits[0].end - splits.splits[0].start;
    req_line->uri.data = splits.splits[1].start;
    req_line->uri.len = splits.splits[1].end - splits.splits[1].start;
    req_line->version.data = splits.splits[2].start;
    req_line->version.len = splits.splits[2].end - splits.splits[2].start;

    free_splits(&splits);
    return HTTP_RES_OK;
}

#endif /* HTTP_TYPES_H */
