#include<stdio.h>
#include<sys/socket.h>

int main(){
    int tcp_socket_fd = socket(
        AF_INET,     // IPv4
        SOCK_STREAM, // TCP
        0 
    );

    if(tcp_socket_fd == -1){
        perror("Failed to create a socket, fd = -1\n");
        return 0;
    }

    printf("Socket created successfully, fd = %d\n", tcp_socket_fd); 
    return 0;
}