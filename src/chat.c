#include "server.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <arpa/inet.h>
#include "server.h"

int main(int argc, char* argv[]){

    int port = atoi(argv[1]); 

    int serverSocketFD                =  socket(AF_INET, SOCK_STREAM, 0); 
    struct sockaddr_in *serverAddress = createAddress(port); 

    int result = bind(serverSocketFD, (struct sockaddr *)serverAddress, sizeof(*serverAddress));

    if (result == 0)
        printf("Socket was bound successfully\n"); 
    else {
        printf("Socket bound fail\n");
        return 1;
    }

    if (listen(serverSocketFD, 10) != 0) {
        perror("Failed to listen on socket");
        return 1;
    }

    start(serverSocketFD);
    shutdown(serverSocketFD, SHUT_RDWR);
    return 0;

}
