#include "server.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <arpa/inet.h>

struct AcceptedSocket acceptedSockets[100];
int acceptedSocketsCount = 0;

struct sockaddr_in* createAddress(int port) {
    // Sólo IPv4 
    struct sockaddr_in  *address = malloc(sizeof(struct sockaddr_in));
    address->sin_family = AF_INET;
    address->sin_port = htons(port);
    address->sin_addr.s_addr = INADDR_ANY;
    return address;
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

void start(int serverSocketFD) {
    while (true) {
        struct AcceptedSocket* clientSocket = accepConnection(serverSocketFD);
        if (clientSocket->acceptedSuccessfully) {
            acceptedSockets[acceptedSocketsCount++] = *clientSocket;
            createReceiveMesssageThread(clientSocket);
        }
        free(clientSocket);
    }
}

void createReceiveMesssageThread(struct AcceptedSocket *pSocket) {
    pthread_t id;
    int *socketFD = malloc(sizeof(int));
    if (socketFD == NULL) {
        perror("Failed to allocate memory for socketFD");
        return;
    }

    *socketFD = pSocket->acceptedSocketFD;

    if (pthread_create(&id, NULL, receiveAndPrintIncomingData, socketFD) != 0) {
        perror("Failed to create thread");
        free(socketFD);
    }
    pthread_detach(id);
}

void *receiveAndPrintIncomingData(void *arg) {
    int socketFD = *(int *)arg;
    free(arg);
    char buffer[1024];

    while (true) {
        ssize_t amountReceived = recv(socketFD, buffer, sizeof(buffer) - 1, 0);

        if (amountReceived > 0) {
            buffer[amountReceived] = '\0';
            printf("Received message: %s\n", buffer);
            sendToGlobalChat(buffer, socketFD);
        }

        if (amountReceived <= 0)
            break;
    }

    close(socketFD);
    return NULL;
}

void sendToGlobalChat(char *buffer, int socketFD) {
    for (int i = 0; i < acceptedSocketsCount; i++) 
        if (acceptedSockets[i].acceptedSocketFD != socketFD) 
            send(acceptedSockets[i].acceptedSocketFD, buffer, strlen(buffer), 0);
        
}



