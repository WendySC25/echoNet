#ifndef SERVER_H
#define SERVER_H

#include <stdbool.h>
#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "user.h"

#include <glib.h>

#define MAX_CONNECTIONS 100

struct Connection {
    int    acceptedSocketFD;
    int    error;
    bool   acceptedSuccessfully;
    FILE   *in;  // Flujo de entrada
    FILE   *out; // Flujo de salida
    User   *user;
};

struct Server {
    int    serverSocketFD;
    struct sockaddr_in* address;
    GHashTable *connections;
    GHashTable *chat_rooms;
    GMutex room_mutex; 
    int    acceptedConnectionCount;
};

struct ThreadData {
    struct Connection *connection;
    struct Server *server;
};


struct sockaddr_in* createAddress(char *ip, int port);
struct Server* newServer(int port);

struct Connection* acceptConnection(struct Server* server);
struct Connection* newConnection(int clientSocketFD);

void startServer(struct Server* server);
void createReceiveMessageThread(struct Connection *pSocket, struct Server* server);
void *receiveMessages(void *arg);
void sendToGlobalChat(char *buffer, int socketFD, struct Server* server);
void sendTo(char *buffer, struct Connection* connection);
void add_connection(struct Server *server, const char *username, struct Connection *conn);
void freeConnection(gpointer data) ;
void remove_connection(struct Server *server, struct Connection* connection);



#endif 
