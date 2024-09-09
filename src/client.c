#include <stdbool.h>
#include <unistd.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include "server.h" 
#include "message.h"
void sentMessageToServer(struct Connection* connection);
void createReceiveMessageThreadClient(struct Connection* connection);
void *listenAndPrint(void *arg);


void sentMessageToServer(struct Connection* connection) {
    char *name = NULL;
    size_t nameSize = 0;
    printf("Please enter your name?\n");
    ssize_t nameCount = getline(&name, &nameSize, stdin);

    if (nameCount > 0) {
        name[nameCount - 1] = '\0';

        // Aqui debe crear un mensaje
        Message message = {
            .type = IDENTIFY,
        };

        strncpy(message.username, name, sizeof(message.username) - 1);
        message.username[sizeof(message.username) - 1] = '\0';

        fprintf(connection->out, "%s\n", toJSON(&message));
        fflush(connection->out);
    }

    char *line = NULL;
    size_t lineSize = 0;
    printf("Type and we will send (type exit)...\n");

    char buffer[1024];

    while (true) {
        ssize_t charCount = getline(&line, &lineSize, stdin);
        if (charCount > 0) {

            line[charCount - 1] = '\0'; 

            if (strcmp(line, "exit") == 0)
                break;

            sprintf(buffer, "%s", line);
            fprintf(connection->out, "%s\n", buffer);
            fflush(connection->out);
        }
    }

    free(name);
    free(line);
}

void createReceiveMessageThreadClient(struct Connection* connection) {
    pthread_t id;
    struct Connection* connPtr = malloc(sizeof(struct Connection));
    if (connPtr == NULL) {
        perror("Failed to allocate memory");
        exit(EXIT_FAILURE);
    }
    *connPtr = *connection; 

    if (pthread_create(&id, NULL, listenAndPrint, connPtr) != 0) {
        perror("Failed to create thread");
        free(connPtr);
    }

    pthread_detach(id);
}

void *listenAndPrint(void *arg) {
    struct Connection* connection = (struct Connection *)arg;

    char buffer[1024];

    while (true) {
        char *result = fgets(buffer, sizeof(buffer) - 1, connection->in);
        
        if (result != NULL) {
            // Remove the newline character if present
            size_t len = strlen(buffer);
            if (len > 0 && buffer[len - 1] == '\n') {
                buffer[len - 1] = '\0';
            }
            printf("Response: %s\n", buffer);
        } else {
            break;
        }
    }

    fclose(connection->in);
    fclose(connection->out);
    close(connection->acceptedSocketFD);
    return NULL;
}

// main
int main(int argc, char* argv[]) {
    if (argc != 3) {
        fprintf(stderr, "Usage: %s <IP> <Port>\n", argv[0]);
        return EXIT_FAILURE;
    }

    int socketFD = socket(AF_INET, SOCK_STREAM, 0);
    if (socketFD < 0) {
        perror("Failed to create socket");
        return EXIT_FAILURE;
    }

    struct sockaddr_in* address = createAddress(argv[1], atoi(argv[2]));
    int result = connect(socketFD, (struct sockaddr *)address, sizeof(*address));
    free(address);

    if (result < 0) {
        perror("Failed to connect");
        close(socketFD);
        return EXIT_FAILURE;
    }

    printf("Connection was successful\n");

    struct Connection* connection = malloc(sizeof(struct Connection));
    if (connection == NULL) {
        perror("Failed to allocate memory for Connection");
        close(socketFD);
        return EXIT_FAILURE;
    }

    connection->acceptedSocketFD = socketFD;
    connection->address = *address; 
    connection->acceptedSuccessfully = (socketFD > 0);
    connection->in = fdopen(socketFD, "r");
    connection->out = fdopen(socketFD, "w");
    connection->user = newUser("newuser");

    if (connection->in == NULL || connection->out == NULL) {
        perror("Failed to create FILE streams");
        free(connection);
        close(socketFD);
        return EXIT_FAILURE;
    }

    createReceiveMessageThreadClient(connection);
    sentMessageToServer(connection);

    fclose(connection->in);
    fclose(connection->out);
    close(connection->acceptedSocketFD);
    free(connection);

    return 0;
}
