#include <stdio.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdbool.h>
#include <fcntl.h>
#include <sys/sendfile.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <limits.h>
#include "utils/string_ops.h"
#include "utils/http_types.h"
#include "utils/http_serve.h"
#include "utils/stat.h"

#define CRLF "\r\n"
#define SPACE " "

/**
 * handle_client_connection: Process incoming client connection
 * @client_socket: File descriptor of the connected client socket
 */
ssize_t handle_client_connection(int client_socket)
{
    ssize_t n = 0;
    char buf[4096];
    string response_404 = string_from_cstr("<h1>404 Not Found!<h1>");

    for (;;)
    {
        memset(buf, 0, sizeof(buf));

        /* Receive incoming data from client socket */
        n = read(client_socket, buf, sizeof(buf) - 1);
        if (n < 0)
        {
            perror("Failed to read the data from the client socket\n");
            return -1;
        }
        if (n == 0)
        {
            printf("Connection closed gracfully\n");
            return 0;
        }
        printf(" - - - - - - - - - - - - - - - - - - - -\n");
        printf("REQUEST: \n%s\n", buf);

        /* Find the first line (request line) terminated by \r\n or \n */
        size_t req_line_len = 0;
        for (size_t i = 0; i < (size_t)n - 1; i++)
        {
            if (buf[i] == '\r' && buf[i + 1] == '\n')
            {
                req_line_len = i;
                break;
            }
            if (buf[i] == '\n')
            {
                req_line_len = i;
                break;
            }
        }

        if (req_line_len == 0)
        {
            printf("Invalid HTTP request, could not find request line terminator\n");
            return -1;
        }

        http_req_line req_line = http_req_line_init();
        http_status result = parse_req_line(buf, req_line_len, &req_line);
        if (result != HTTP_RES_OK)
        {
            printf("Failed to parse request line\n");
            return -1;
        }

        string root_route = string_from_cstr("/");
        if (strings_equal(req_line.uri, root_route))
        {
            if (!http_serve_file(client_socket, string_from_cstr("./www/index.html")))
            {
                printf("Failed to serve index.html for route %.*s\n", (int)root_route.len, root_route.data);
                return -1;
            };
        }
        else
        {
            printf("Unknown route requested: %.*s\n", (int)req_line.uri.len, req_line.uri.data);
            if (!http_send_response(
                    client_socket,
                    http_response_generate(buf, sizeof(buf), HTTP_RES_NOT_FOUND, response_404.len),
                    response_404))
            {
                printf("Failed to send 404 response\n");
                return -1;
            }
        }

        (void)
            close(client_socket);
        break;
    }
    printf(" - - - - - - - - - - - - - - - - - - - -\n");

    return 0;
}

/**
 * Creates a TCP socket, binds to port 8000, and accepts incoming connections.
 * Returns 0 on success, 1 on error.
 */
int main()
{
    int ret = 0;
    int rc = 0;
    int tcp_socket = 0;
    int ls = 0;
    int client_socket = 0;
    struct sockaddr_in bind_addr;
    int optionsEnabled = 1;

    const char *web_root = "./www";
    printf("Web root directory: %s\n", web_root);

    fs_metadata web_root_metadata = fs_get_metadata(string_to_view(string_from_cstr(web_root)));
    if (!web_root_metadata.exists)
    {
        fprintf(stderr, "Web root directory '%s' does not exist. Creating it now.\n", web_root);
        mkdir(web_root, 0300);
    }

    memset(&bind_addr, 0, sizeof(bind_addr));

    /* Create TCP socket (AF_INET=IPv4, SOCK_STREAM=TCP) */
    tcp_socket = socket(AF_INET, SOCK_STREAM, 0);

    if (tcp_socket < 0)
    {
        perror("Failed to create a socket, fd = -1\n");
        return 0;
    }

    printf("Socket created successfully, fd = %d\n", tcp_socket);

    (void)setsockopt(tcp_socket, SOL_SOCKET, SO_REUSEADDR, &optionsEnabled, sizeof(optionsEnabled));

    /* Configure socket address for binding to port 8000 */
    bind_addr.sin_family = AF_INET;         /* IPv4 */
    bind_addr.sin_port = htons(8000);       /* Port 8000 */
    bind_addr.sin_addr.s_addr = INADDR_ANY; /* Listen on all interfaces */

    rc = bind(tcp_socket, (const struct sockaddr *)&bind_addr, sizeof(bind_addr));
    if (rc < 0)
    {
        perror("Failed to bind the socket\n");
        ret = 1;
        goto exit;
    }

    /* Mark socket as passive to accept incoming connections */
    ls = listen(tcp_socket, SOMAXCONN);
    if (ls < 0)
    {
        perror("Failed to create the listener\n");
        ret = 1;
        goto exit;
    }
    printf("Listener succeeded\n");

    /* Accept and handle incoming client connections */
    for (;;)
    {
        printf("Waiting for connection\n");
        client_socket = accept(tcp_socket, NULL, NULL);
        if (client_socket < 0)
        {
            perror("Failed to create the client connection\n");
            ret = 1;
            goto exit;
        }
        printf("Received a connection %d\n", client_socket);
        rc = handle_client_connection(client_socket);
    }

exit:
    close(tcp_socket);
    return ret;
}