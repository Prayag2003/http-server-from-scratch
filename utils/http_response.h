#ifndef HTTP_RESPONSE_H
#define HTTP_RESPONSE_H

#include <stdbool.h>
#include "http_common.h"

/**
 * Convert http_status enum to string representation
 */
static inline const char *http_status_to_str(http_status status)
{
    switch (status)
    {
    case HTTP_RES_OK:
        return "OK";
    case HTTP_RES_INTERNAL_SERVER_ERR:
        return "Internal Server Error";
    case HTTP_RES_BAD_REQUEST:
        return "Bad Request";
    case HTTP_RES_NOT_FOUND:
        return "Not Found";
    default:
        return "Unknown Status";
    }
}

/**
 * Generate HTTP response header
 * @buf: Buffer to write the response header into
 * @buf_len: Size of the buffer
 * @status: HTTP status code
 * @body_len: Length of the response body
 * @return: String containing the generated header
 */
string http_response_generate(char *buf, size_t buf_len, http_status status, size_t body_len);

/**
 * Send HTTP response (header + body) over socket
 * @socket: Socket file descriptor
 * @header: Response header string
 * @body: Response body string
 * @return: true on success, false on failure
 */
bool http_send_response(int socket, string header, string body);

#endif /* HTTP_RESPONSE_H */
