#include "server.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <arpa/inet.h>

struct Connection connections[100];
int acceptedConnectionCount = 0;

struct sockaddr_in* createAddress(int port) {
    // Sólo IPv4 
    struct sockaddr_in  *address = malloc(sizeof(struct sockaddr_in));
    address->sin_family = AF_INET;
    address->sin_port = htons(port);
    address->sin_addr.s_addr = INADDR_ANY;
    return address;
}


// Esto es como el constructor de la clase conexión
struct Connection* accepConnection(int serverSocketFD) {
    struct sockaddr_in clientAddress;
    socklen_t clientAddressSize = sizeof(clientAddress);
    int clientSocketFD = accept(serverSocketFD, (struct sockaddr *)&clientAddress, &clientAddressSize);
    struct Connection* connection = malloc(sizeof(struct Connection));
    if (connection == NULL) {
        perror("Failed to allocate memory for Connection. ");
        return NULL;
    }
    connection->address = clientAddress;
    connection->acceptedSocketFD = clientSocketFD;
    connection->acceptedSuccessfully = clientSocketFD > 0;

    if (!connection->acceptedSuccessfully)
        connection->error = clientSocketFD;

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

// el método sirve de la clase Servidor
void start(int serverSocketFD) {
    while (true) {
        struct Connection* clientSocket = accepConnection(serverSocketFD);
        if (clientSocket->acceptedSuccessfully) {
            connections[acceptedConnectionCount++] = *clientSocket;
            createReceiveMesssageThread(clientSocket);
        }
        free(clientSocket);
    }
}

// Esto esta dentro del método sirve, lo que hace es crear el hilo de ejecucion de recibir mensajes
void createReceiveMesssageThread(struct Connection *pSocket) {
    pthread_t id;
    struct Connection *socketCopy = malloc(sizeof(struct Connection));
    if (socketCopy == NULL) {
        perror("Failed to allocate memory for socketFD");
        return;
    }

    *socketCopy = *pSocket;

    if (pthread_create(&id, NULL, receiveAndPrintIncomingData, socketCopy) != 0) {
        perror("Failed to create thread");
        free(socketCopy);
    }
    pthread_detach(id);
}

// Método recibe mensajes de la clase Conexión
void *receiveAndPrintIncomingData(void *arg) {
    struct Connection *socket = (struct Connection *)arg;
    char buffer[1024];

    while (true) {
        if (fgets(buffer, sizeof(buffer), socket->in) != NULL) {
            printf("Received message: %s", buffer);
            sendToGlobalChat(buffer, socket->acceptedSocketFD);
        } else {
            break; 
        }
    }

    fclose(socket->in);
    fclose(socket->out);
    close(socket->acceptedSocketFD);
    free(socket);
    return NULL;
}

void sendToGlobalChat(char *buffer, int socketFD) {
    for (int i = 0; i < acceptedConnectionCount; i++) 
        if (connections[i].acceptedSocketFD != socketFD) {
            fprintf(connections[i].out, "%s", buffer);
            fflush(connections[i].out);  
        }
}



