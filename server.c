#include <stdio.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <string.h>

int main()
{
    /* declaration */
    int ret = 0;
    int rc = 0;
    int tcp_socket = 0;
    int ls = 0;
    int client_socket = 0;
    struct sockaddr_in bind_addr;

    /* initialization */
    memset(&bind_addr, 0, sizeof(bind_addr));

    // socket functions returns a FD
    tcp_socket = socket(
        AF_INET,     // IPv4
        SOCK_STREAM, // TCP
        0);

    if (tcp_socket < 0)
    {
        perror("Failed to create a socket, fd = -1\n");
        return 0;
    }

    printf("Socket created successfully, fd = %d\n", tcp_socket);

    /*
        The TCP Socket needs an address, so we use the bind() to connect to an address
    */
    bind_addr.sin_port = htons(8000);
    bind_addr.sin_family = AF_INET;
    bind_addr.sin_addr.s_addr = INADDR_ANY;

    rc = bind(tcp_socket, (const struct sockaddr *)&bind_addr, sizeof(bind_addr));
    if (rc < 0)
    {
        perror("Failed to bind the socket\n");
        ret = 1;
        goto exit;
    }

    /*
        listen()  marks  the  socket  referred to by sockfd as a passive socket, that is, as a socket that will be used to accept incoming connection requests using accept(2)
    */
    ls = listen(tcp_socket, SOMAXCONN);
    if (ls < 0)
    {
        perror("Failed to create the listener\n");
        ret = 1;
        goto exit;
    }
    printf("Listener succeeded\n");

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
    }

exit:
    close(tcp_socket);
    return ret;
}