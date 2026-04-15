#ifndef HTTP_RESPONSE_H
#define HTTP_RESPONSE_H

#include "http_common.h"

typedef struct
{
    const char *version;
    http_status status_code;
    const char *reason;
} http_result;

const char *http_status_to_str(http_status status)
{
    switch (status)
    {
    case HTTP_RES_OK:
        return "OK";
    case HTTP_RES_INTERNAL_SERVER_ERR:
        return "Internal Server Error";
    case HTTP_RES_BAD_REQUEST:
        return "Bad Request";
    default:
        return "Unknown Status";
    }
}

#endif /* HTTP_RESPONSE_H */
