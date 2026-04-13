#include <stdio.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <string.h>

/**
 * handle_client_connection: Process incoming client connection
 * @client_socket: File descriptor of the connected client socket
 *
 * Receives data from the client socket and processes the request.
 * Returns the number of bytes received, or -1 on error (ssize_t)
 */
ssize_t handle_client_connection(int client_socket)
{
    ssize_t n = 0;
    char buffer[4096];

    for (;;)
    {
        memset(buffer, 0, sizeof(buffer));

        /* Receive incoming data from client socket */
        n = read(client_socket, buffer, sizeof(buffer) - 1);
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
        printf("Buffer => %s\n", buffer);
        printf(" - - - - - - - - - - - - - - - - - - - -\n");
    }

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