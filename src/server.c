#include "server.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <arpa/inet.h>

struct AcceptedSocket acceptedSockets[10];
int acceptedSocketsCount = 0;

struct sockaddr_in* createAddress(int port) {
    // Sólo IPv4 
    struct sockaddr_in  *address = malloc(sizeof(struct sockaddr_in));
    address->sin_family = AF_INET;
    address->sin_port = htons(port);
    address->sin_addr.s_addr = INADDR_ANY;
    return address;
}


void start(int serverSocketFD) {
    while (true) {
        struct AcceptedSocket* clientSocket = accepConnection(serverSocketFD);
        if (clientSocket->acceptedSuccessfully) 
            acceptedSockets[acceptedSocketsCount++] = *clientSocket;
        free(clientSocket);
    }
}

struct AcceptedSocket* accepConnection(int serverSocketFD) {
    struct sockaddr_in clientAddress;
    socklen_t clientAddressSize = sizeof(clientAddress);
    int clientSocketFD = accept(serverSocketFD, (struct sockaddr *)&clientAddress, &clientAddressSize);

    struct AcceptedSocket* acceptedSocket = malloc(sizeof(struct AcceptedSocket));
    if (acceptedSocket == NULL) {
        perror("Failed to allocate memory for AcceptedSocket. ");
        return NULL;
    }
    acceptedSocket->address = clientAddress;
    acceptedSocket->acceptedSocketFD = clientSocketFD;
    acceptedSocket->acceptedSuccessfully = clientSocketFD > 0;

    if (!acceptedSocket->acceptedSuccessfully)
        acceptedSocket->error = clientSocketFD;

    return acceptedSocket;
}
