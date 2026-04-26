#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include "http_response.h"

#define CRLF "\r\n"

string http_response_generate(char *buf, size_t buf_len, http_status status, size_t body_len)
{
    string response;
    response.len = 0;
    memset(buf, 0, buf_len);

    response.len += sprintf(buf, "%s %d %s" CRLF, "HTTP/1.1", status, http_status_to_str(status));
    response.len += sprintf(buf + response.len, "Content-Length: %zu" CRLF, body_len);
    response.len += sprintf(buf + response.len, CRLF);
    response.data = buf;

    return response;
}

bool http_send_response(int socket, string header, string body)
{
    ssize_t n = send(socket, header.data, header.len, MSG_MORE);
    if (n < 0)
    {
        perror("Failed to send response header\n");
        return false;
    }
    if (n == 0)
    {
        fprintf(stderr, "send() returned 0 while sending response header\n");
        return false;
    }

    n = send(socket, body.data, body.len, 0);
    return true;
}
