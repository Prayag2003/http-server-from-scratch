#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/sendfile.h>
#include <sys/stat.h>
#include <limits.h>
#include "http_serve.h"
#include "http_response.h"
#include "stat.h"

bool http_serve_file(int client_socket, string filename)
{
    char file_buf[PATH_MAX];
    char header_buf[PATH_MAX];
    string header;
    bool return_value = false;
    int in_fd = -1;

    fs_metadata metadata = fs_get_metadata(string_to_view(filename));
    if (!metadata.exists)
    {
        memset(header_buf, 0, sizeof(header_buf));
        (void)http_send_response(
            client_socket,
            http_response_generate(header_buf, sizeof(header_buf), HTTP_RES_NOT_FOUND, strlen("Error 404: Not Found")),
            string_from_cstr("Error 404: Not Found"));
        return_value = true;
        goto cleanup;
    }

    memset(file_buf, 0, sizeof(file_buf));
    memcpy(file_buf, filename.data, filename.len);

    memset(header_buf, 0, sizeof(header_buf));
    header = http_response_generate(header_buf, sizeof(header_buf), HTTP_RES_OK, metadata.file_size);

    ssize_t n = send(client_socket, header.data, header.len, MSG_MORE);
    if (n < 0)
    {
        perror("Failed to send response header\n");
        return_value = false;
        goto cleanup;
    }
    if (n == 0)
    {
        fprintf(stderr, "send() returned 0 while sending response header\n");
        return_value = false;
        goto cleanup;
    }

    in_fd = open(file_buf, O_RDONLY);
    if (in_fd < 0)
    {
        memset(header_buf, 0, sizeof(header_buf));
        http_send_response(
            client_socket,
            http_response_generate(header_buf, sizeof(header_buf), HTTP_RES_INTERNAL_SERVER_ERR, strlen("Error 500: Internal Server Error")),
            string_from_cstr("Error 500: Internal Server Error"));
        perror("Failed to open file for reading\n");
        goto cleanup;
    }

    // Use sendfile(2) to send the file without using userspace buffers for efficiency, we save the overhead of copying file contents into user space and then back to kernel space for sending over the socket.
    ssize_t result = 0;
    int sent = 0;

    while (sent < metadata.file_size)
    {
        result = sendfile(client_socket, in_fd, NULL, metadata.file_size - sent);
        if (result < 0)
        {
            printf("Failed to send file contents for %s\n", file_buf);
            memset(header_buf, 0, sizeof(header_buf));
            http_send_response(
                client_socket,
                http_response_generate(header_buf, sizeof(header_buf), HTTP_RES_INTERNAL_SERVER_ERR, strlen("Error 500: Internal Server Error")),
                string_from_cstr("Error 500: Internal Server Error"));
            return_value = false;
            goto cleanup;
        }
        sent += result;
    }

    /* File served successfully */
    return_value = true;

cleanup:
    if (in_fd != -1)
        close(in_fd);
    return return_value;
}
