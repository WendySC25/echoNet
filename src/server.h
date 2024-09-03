#ifndef SERVER_H
#define SERVER_H

#include <stdbool.h>
#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

struct Connection {
    int    acceptedSocketFD;
    struct sockaddr_in address;
    int    error;
    bool   acceptedSuccessfully;
    FILE   *in;  // Flujo de entrada
    FILE   *out; // Flujo de salida
};

struct sockaddr_in* createAddress(int port);
struct Connection* accepConnection(int serverSocketFD);
void start(int serverSocketFD);
void createReceiveMesssageThread(struct Connection *pSocket);
void *receiveAndPrintIncomingData(void *arg);
void sendToGlobalChat(char *buffer, int socketFD);

#endif 
