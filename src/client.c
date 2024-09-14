#include <stdbool.h>
#include <unistd.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include <glib.h>
#include "server.h" 
#include "message.h"
#include <signal.h>
#include "clientInterface.h"
#include "client.h"

volatile sig_atomic_t signal_received = 0;  


struct Connection* connectNewClient(char *ip, int port){

    int socketFD = socket(AF_INET, SOCK_STREAM, 0);
    if (socketFD < 0) {
        perror("Failed to create socket");
        return NULL;
    }

    struct sockaddr_in* address = createAddress(ip, port);
    int result = connect(socketFD, (struct sockaddr *)address, sizeof(*address));
    free(address);
    if (result < 0) {
        perror("Failed to connect");
        close(socketFD);
        return NULL;
    }

    struct Connection* connection = newConnection(socketFD);
    return connection;

}

void disconnect(struct Connection* connection) {
    if (connection != NULL) {
        // Send a disconnect message to the server
        Message msg;
        msg.type = DISCONNECT;
        char buffer[1024];
        snprintf(buffer, sizeof(buffer), "%s\n", toJSON(&msg));
        fprintf(connection->out, "%s\n", buffer);
        fflush(connection->out);

        freeConnection(connection);

        printf("Disconnected successfully.\n");
        printByeHeader();
            
        exit(EXIT_SUCCESS);
    }
}


void sentMessageToServer(struct Connection* connection) {
    char *line = NULL;
    size_t lineSize = 0;

    char buffer[1024];

    while (true) {

        if (signal_received) {
            disconnect(connection);
        }

        ssize_t charCount = getline(&line, &lineSize, stdin);
        if (charCount > 0) {
            line[charCount - 1] = '\0'; 
            Message message = parseInput(line);
            sprintf(buffer, "%s", toJSON(&message));
            fprintf(connection->out, "%s\n", buffer);
            fflush(connection->out);

            if (strcmp(line, "\\bye") == 0) { 
                disconnect(connection);
            }

        }
    }

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

    if (pthread_create(&id, NULL, receiveMessageFromServer, connPtr) != 0) {
        perror("Failed to create thread");
        free(connPtr);
    }
    pthread_detach(id);
}

     
void handleReceiveMessage(char* buffer, struct Connection* conection) {

    printf("MENSAJE DEL SERVIDOR: %s \n", buffer);
    Message message = getMessage(buffer);

    switch (message.type) {
        case NEW_USER:
            handleNewUser(message.username);
            break;

        case TEXT_FROM:
            handlePrivateMessage(message.username, message.text);
            break;
        
        case PUBLIC_TEXT_FROM:
            handlePublicMessage(message.username, message.text);
            break;
        
        case NEW_STATUS:
            handleNewStatus(message.username, UserStatusToString(message.status));
            break;
        
        case USER_LIST:
            handleUserList(message.connections);
            break;
        
        case RESPONSE:
            handleServerResponse(message, conection);
            break;

        case INVITATION:
            handleInvitation(message.username, message.roomname);
            break;

        case JOINED_ROOM:
            handleJoinedRoom(message.username, message.roomname);
            break;
        
        case ROOM_TEXT_FROM:
            handleRoomText(message.username, message.roomname, message.text);
            break; 

        case ROOM_USER_LIST:
            handleRoomUserList(message.chat_rooms, message.roomname);
            break;
        
        case LEFT_ROOM:
            handleLeftRoom(message.username, message.roomname);
            break;

        case DISCONNECTED:
            handleDisconnection(message.username);
            break;
            
        default:
            // Invalid Message is ignore
            break;
    }

    if(message.type == RESPONSE && message.operation == INVALID)
        disconnect(conection);
}

void *receiveMessageFromServer(void *arg) {
    struct Connection* connection = (struct Connection *)arg;
    char buffer[1024];
    while (true) {
        char *result = fgets(buffer, sizeof(buffer) - 1, connection->in);
        if (result != NULL) 
            handleReceiveMessage(buffer, connection);
        else 
            break;
    }   

    return NULL;
}

void handle_sigint(int sig) {
    printf("\nSIGINT. Closing connection...\n");
    signal_received = 1; 
}
