#include<stdio.h>
#include<sys/socket.h>

int main(){
    int tcp_socket = socket(
        AF_INET,  // IPv4
        SOCK_STREAM, // TCP
        0 
    );

    printf("%d\n", tcp_socket);
}