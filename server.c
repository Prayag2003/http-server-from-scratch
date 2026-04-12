#include <stdio.h>
#include <sys/socket.h>
#include <arpa/inet.h>

int main(){
    // socket functions returns a FD
    int tcp_socket = socket(
        AF_INET,     // IPv4
        SOCK_STREAM, // TCP
        0 
    );

    if(tcp_socket < 0){
        perror("Failed to create a socket, fd = -1\n");
        return 0;
    }

    printf("Socket created successfully, fd = %d\n", tcp_socket); 

    /*
        The TCP Socket needs an address, so we use the bind() to connect to an address
    */ 
    struct sockaddr_in bind_addr;
    memset(&bind_addr, 0, sizeof(bind_addr));

    bind_addr.sin_port = htons(8000);
    bind_addr.sin_family = AF_INET;
    bind_addr.sin_addr.s_addr = INADDR_ANY;

    int rc = bind(tcp_socket, &bind_addr, sizeof(bind_addr));
    if(rc < 0){
        perror("Failed to bind the socket\n");
        return 0;
    }

    /*
        listen()  marks  the  socket  referred to by sockfd as a passive socket, that is, as a socket that will be used to accept incoming connection requests using accept(2)
    */  
    int ls = listen(tcp_socket, SOMAXCONN);
    if(ls < 0){
        perror("Failed to create the listener\n");
        return 0;
    }

    printf("Listener succeeded\n");

    exit: 
        close(tcp_socket);
    return 0;
}