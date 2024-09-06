#include "server.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>

struct sockaddr_in* createAddress(int port) {
    struct sockaddr_in *address = malloc(sizeof(struct sockaddr_in));
    if (address == NULL) {
        perror("Failed to allocate memory for address");
        return NULL;
    }
    address->sin_family = AF_INET;
    address->sin_port = htons(port);
    address->sin_addr.s_addr = INADDR_ANY;
    return address;
}

struct Server* newServer(int port) {
    struct Server* server = malloc(sizeof(struct Server));

    if (server == NULL) {
        perror("Failed to allocate memory for Server");
        return NULL;
    }

    server->serverSocketFD = socket(AF_INET, SOCK_STREAM, 0); 
    if (server->serverSocketFD < 0) {
        perror("Failed to create socket");
        free(server);
        return NULL;
    }

    server->address = createAddress(port);
    if (server->address == NULL) {
        close(server->serverSocketFD);
        free(server);
        return NULL;
    }

    int result = bind(server->serverSocketFD, (struct sockaddr *)server->address, sizeof(*server->address));
    if (result != 0) {
        perror("Failed to bind socket");
        close(server->serverSocketFD);
        free(server->address);
        free(server);
        return NULL;
    }

    if (listen(server->serverSocketFD, 10) != 0) {
        perror("Failed to listen on socket");
        close(server->serverSocketFD);
        free(server->address);
        free(server);
        return NULL;
    }

    server->acceptedConnectionCount = 0;
    return server;
}

struct Connection* acceptConnection(struct Server* server) {
    struct sockaddr_in clientAddress;
    socklen_t clientAddressSize = sizeof(clientAddress);
    int clientSocketFD = accept(server->serverSocketFD, (struct sockaddr *)&clientAddress, &clientAddressSize);

    if (clientSocketFD < 0) {
        perror("Failed to accept connection");
        return NULL;
    }

    struct Connection* connection = malloc(sizeof(struct Connection));
    if (connection == NULL) {
        perror("Failed to allocate memory for Connection");
        close(clientSocketFD);
        return NULL;
    }

    connection->address = clientAddress;
    connection->acceptedSocketFD = clientSocketFD;
    connection->acceptedSuccessfully = clientSocketFD > 0;

    if (!connection->acceptedSuccessfully) {
        connection->error = clientSocketFD;
    }

    connection->in  = fdopen(clientSocketFD, "r");
    connection->out = fdopen(clientSocketFD, "w");

    if (connection->in == NULL || connection->out == NULL) {
        perror("Failed to create FILE streams");
        close(clientSocketFD);
        free(connection);
        return NULL;
    }

    return connection;
}

void startServer(struct Server* server) {
    while (1) {
        struct Connection* clientSocket = acceptConnection(server);
        if (clientSocket && clientSocket->acceptedSuccessfully) {
            if (server->acceptedConnectionCount < MAX_CONNECTIONS) {
                server->connections[server->acceptedConnectionCount++] = *clientSocket;
                createReceiveMessageThread(clientSocket, server);
            } else {
                printf("Maximum number of connections reached.\n");
                free(clientSocket);
            }
        } else if (clientSocket) {
            free(clientSocket);
        }
    }
}

void createReceiveMessageThread(struct Connection *pSocket, struct Server* server) {
    pthread_t id;
    struct ThreadData *threadData = malloc(sizeof(struct ThreadData));
    if (threadData == NULL) {
        perror("Failed to allocate memory for threadData");
        return;
    }

    threadData->connection = pSocket;
    threadData->server = server;

    if (pthread_create(&id, NULL, receiveAndPrintIncomingData, threadData) != 0) {
        perror("Failed to create thread");
        free(threadData);
    }
    pthread_detach(id);
}

void *receiveAndPrintIncomingData(void *arg) {
    struct ThreadData *data = (struct ThreadData *)arg;
    struct Connection *socket = data->connection;
    struct Server *server = data->server;

    char buffer[1024];

    while (1) {
        if (fgets(buffer, sizeof(buffer), socket->in) != NULL) {
            printf("Received message: %s", buffer);
            sendToGlobalChat(buffer, socket->acceptedSocketFD, server);
        } else {
            break; 
        }
    }

    fclose(socket->in);
    fclose(socket->out);
    close(socket->acceptedSocketFD);
    free(data);
    return NULL;
}

void sendToGlobalChat(char *buffer, int socketFD, struct Server* server) {
    for (int i = 0; i < server->acceptedConnectionCount; i++) {
        if (server->connections[i].acceptedSocketFD != socketFD) {
            fprintf(server->connections[i].out, "%s", buffer);
            fflush(server->connections[i].out);  
        }
    }
}
