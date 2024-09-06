#ifndef SERVER_H
#define SERVER_H

#include <stdbool.h>
#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define MAX_CONNECTIONS 100

struct Connection {
    int    acceptedSocketFD;
    struct sockaddr_in address;
    int    error;
    bool   acceptedSuccessfully;
    FILE   *in;  // Flujo de entrada
    FILE   *out; // Flujo de salida
};

struct Server {
    int    serverSocketFD;
    struct sockaddr_in* address;
    struct Connection connections[MAX_CONNECTIONS];
    int    acceptedConnectionCount;
};

struct ThreadData {
    struct Connection *connection;
    struct Server *server;
};


struct sockaddr_in* createAddress(int port);
struct Server* newServer(int port);

struct Connection* acceptConnection(struct Server* server);

void startServer(struct Server* server);
void createReceiveMessageThread(struct Connection *pSocket, struct Server* server);
void *receiveAndPrintIncomingData(void *arg);
void sendToGlobalChat(char *buffer, int socketFD, struct Server* server);



#endif 
