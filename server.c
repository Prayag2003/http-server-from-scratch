#include <stdio.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdbool.h>
#include <signal.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <limits.h>
#include <stdint.h>
#include <pthread.h>
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
void *handle_client_connection(void *client_socket_ptr)
{
    int client_socket = (int)(intptr_t)client_socket_ptr;
    ssize_t n = 0;
    int result = 0;
    char buf[4096];
    string response_404 = string_from_cstr("<h1>404 Not Found!<h1>");

    printf("- - - - - - - - - - New Connection - - - - - - - - - -\n");
    for (;;)
    {
        memset(buf, 0, sizeof(buf));

        /* Receive incoming data from client socket */
        n = read(client_socket, buf, sizeof(buf) - 1);
        if (n < 0)
        {
            perror("Failed to read the data from the client socket\n");
            result = -1;
            break;
        }
        if (n == 0)
        {
            printf("Connection closed gracfully\n");
            break;
        }
        // printf("REQUEST: \n%s\n", buf);

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
            result = -1;
            break;
        }

        http_req_line req_line = http_req_line_init();
        http_status result = parse_req_line(buf, req_line_len, &req_line);

        printf("Parsed request line: method=%.*s, uri=%.*s, version=%.*s\n",
               (int)req_line.method.len, req_line.method.data,
               (int)req_line.uri.len, req_line.uri.data,
               (int)req_line.version.len, req_line.version.data);

        if (result != HTTP_RES_OK)
        {
            printf("Failed to parse request line\n");
            break;
        }

        string root_route = string_from_cstr("/");
        string assets_prefix = string_from_cstr("/assets/");

        if (strings_equal(req_line.uri, root_route))
        {
            if (!http_serve_file(client_socket, string_from_cstr("./www/index.html")))
            {
                printf("Failed to serve index.html for route %.*s\n", (int)root_route.len, root_route.data);
                result = -1;
                break;
            };
        }
        else if (req_line.uri.len > assets_prefix.len &&
                 strncmp(req_line.uri.data, assets_prefix.data, assets_prefix.len) == 0)
        {
            /* Serve static files from /assets/ */
            char asset_path[PATH_MAX];
            memset(asset_path, 0, sizeof(asset_path));
            snprintf(asset_path, sizeof(asset_path), ".%.*s", (int)req_line.uri.len, req_line.uri.data);

            if (!http_serve_file(client_socket, string_from_cstr(asset_path)))
            {
                printf("Failed to serve asset %s\n", asset_path);
                result = -1;
                break;
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
                result = -1;
                break;
            }
        }
    }
    printf("Closing connection %d\n", client_socket);
    (void)
        close(client_socket);
    printf(" - - - - - - - - - - - - - - - - - - - -\n");

    pthread_exit((void *)(intptr_t)result);
}

/**
 * Creates a TCP socket, binds to port 8000, and accepts incoming connections.
 * Returns 0 on success, 1 on error.
 */
int main()
{
    //  - - - Declarations and initializations - - -
    int exit_code = 0;
    int tcp_socket = 0;
    int bind_result = 0;
    int client_socket = 0;
    int listen_result = 0;
    int reuseaddr_enabled = 1;
    struct sockaddr_in bind_addr;
    const int LISTENER_PORT = 8000;
    pthread_t *threads = NULL;
    size_t thread_count = 0;
    size_t thread_capacity = 10;
    threads = calloc(thread_capacity, sizeof(pthread_t));

    // - - - Ensure web root directory exists - - -
    const char *web_root = "./www";
    printf("Web root directory: %s\n", web_root);

    fs_metadata web_root_metadata = fs_get_metadata(string_to_view(string_from_cstr(web_root)));
    if (!web_root_metadata.exists)
    {
        fprintf(stderr, "Web root directory '%s' does not exist. Creating it now.\n", web_root);
        mkdir(web_root, 0300);
    }

    memset(&bind_addr, 0, sizeof(bind_addr));

    /* Create TCP socket */
    tcp_socket = socket(
        AF_INET,     // IPv4
        SOCK_STREAM, // TCP socket
        0);

    if (tcp_socket < 0)
    {
        perror("Failed to create a socket, fd = -1\n");
        return 0;
    }

    printf("Socket created successfully, fd = %d\n", tcp_socket);

    /* Set socket options to allow address reuse */
    (void)setsockopt(
        tcp_socket, SOL_SOCKET,
        SO_REUSEADDR,
        &reuseaddr_enabled, sizeof(reuseaddr_enabled));

    /* Configure socket address for binding to port 8000 */
    bind_addr.sin_family = AF_INET;            /* IPv4 */
    bind_addr.sin_port = htons(LISTENER_PORT); /* Port 8000 */
    bind_addr.sin_addr.s_addr = INADDR_ANY;    /* Listen on all interfaces */

    bind_result = bind(tcp_socket, (const struct sockaddr *)&bind_addr, sizeof(bind_addr));
    if (bind_result < 0)
    {
        perror("Failed to bind the socket\n");
        exit_code = 1;
        goto exit;
    }

    /* Mark socket as passive to accept incoming connections */
    listen_result = listen(tcp_socket, SOMAXCONN);
    if (listen_result < 0)
    {
        perror("Failed to create the listener\n");
        exit_code = 1;
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
            exit_code = 1;
            goto exit;
        }

        printf("Received a connection %d\n", client_socket);

        pthread_t thread;
        // bind_result = handle_client_connection(client_socket);
        bind_result = pthread_create(&thread, NULL, handle_client_connection, (void *)(intptr_t)client_socket);
        if (bind_result != 0)
        {
            perror("Failed to create thread for client connection\n");
            continue;
        }

        threads[thread_count++] = thread;
        if (thread_count + 1 > thread_capacity)
        {
            thread_capacity *= 2;
            pthread_t *new_threads = realloc(threads, thread_capacity * sizeof(pthread_t));
            if (!new_threads)
            {
                /* Thread is still running, just not tracked. Reset count to avoid overflow. */
                fprintf(stderr, "Warning: Failed to reallocate threads array, but server continues\n");
                thread_count = 0;
            }
            else
            {
                threads = new_threads;
            }
        }
    }

exit:
    printf("Waiting for all threads to complete...\n");
    for (size_t i = 0; i < thread_count; i++)
    {
        pthread_join(threads[i], NULL);
    }
    if (threads)
    {
        free(threads);
    }
    close(tcp_socket);
    return exit_code;
}