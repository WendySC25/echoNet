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

void sentMessageToServer(struct Connection* connection);
void createReceiveMessageThreadClient(struct Connection* connection);
void *receiveMessageFromServer(void *arg);
struct Connection* connetNewClient(char *ip, int port);
void handleReciveMessage(char* buffer, struct Connection* conection);
void handleNewUser(const char *username);
void handleServerResponse(Message msg);
void handlePublicMessage(const char *username, const char *text);
void handleNewStatus(const char *username, const char *status);
void handleUserList(GHashTable *connections);
void handleInvitation(const char *username, const char *roomname, struct Connection* connection);
void handleJoinedRoom(const char *username, const char *roomname);
void handleRoomText(const char *username, const char *roomname, const char *text);
void handleRoomUserList(GHashTable *connections, const char *roomname);
void handleLeftRoom(const char *username, const char *roomname );


struct Connection* connetNewClient(char *ip, int port){

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

void sentMessageToServer(struct Connection* connection) {

    //REVISAR
    // char *name = NULL;
    // size_t nameSize = 0;
    // printf("Please enter your name?\n");
    // ssize_t nameCount = getline(&name, &nameSize, stdin);

    // if (nameCount > 0) {
    //     name[nameCount - 1] = '\0';

    //     // Aqui debe crear un mensaje
    //     Message message = {
    //         .type = IDENTIFY,
    //     };

    //     strncpy(message.username, name, sizeof(message.username) - 1);
    //     message.username[sizeof(message.username) - 1] = '\0';

    //     connection->user = newUser(name);

    //     fprintf(connection->out, "%s\n", toJSON(&message));
    //     fflush(connection->out);
    // }

    char *line = NULL;
    size_t lineSize = 0;
    printf("Type and we will send (type exit)...\n");

    char buffer[1024];

    while (true) {
        ssize_t charCount = getline(&line, &lineSize, stdin);
        if (charCount > 0) {

            line[charCount - 1] = '\0'; 

            // CAMBIA TOTALMENTE EL ENVIO DE MENSAJES
            Message message = parseInput(line);
            sprintf(buffer, "%s", toJSON(&message));
            fprintf(connection->out, "%s\n", buffer);
            fflush(connection->out);

            if (strcmp(line, "\\bye") == 0) { 
                //ESTO ESTÁ MAL, SOLO ES UN CAMBIO
                exit(EXIT_FAILURE);
            }

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

    if (pthread_create(&id, NULL, receiveMessageFromServer, connPtr) != 0) {
        perror("Failed to create thread");
        free(connPtr);
    }
    pthread_detach(id);
}


void handleNewUser(const char *username) {
    //Andir nuevo ususario a la lista de usuarios conectados.
    printf("%s enter the chat.\n", username);
}

void handleNewStatus(const char *username, const char *status) {
    //Andir nuevo ususario a la lista de usuarios conectados.
    printf("%s change status to: %s.\n", username, status);
}

void handleServerResponse(Message msg) {
   
    printf("%s \n", toJSON(&msg));
}

void handlePrivateMessage(const char *username, const char *text){
    printf(" %s whispered to you: %s\n", username, text);
}

void handlePublicMessage(const char *username, const char *text){
    printf(" %s sent to global chat: %s\n", username, text);
}

void handleDisconnection(const char *username){
    printf(" %s leave the chat :( \n", username);
}

void handleUserList(GHashTable *connections) {

    if (g_hash_table_size(connections) == 0) {
        printf("No hay usuarios conectados. Eres tú y la soledad :( \n");
        return;
    }

    GHashTableIter iter;
    gpointer key, value;
    g_hash_table_iter_init(&iter, connections);
    while (g_hash_table_iter_next(&iter, &key, &value)) {
        printf("Username: %s, Status: %s\n", (char *)key, (char *)value);
    }
    g_hash_table_destroy(connections);

}

void handleInvitation(const char *username, const char *roomname, struct Connection* connection){
    printf("%s invite you to a room %s\n", username, roomname );
    Message msg = { .type = JOIN_ROOM };

    strncpy(msg.roomname, roomname, sizeof(msg.roomname) - 1);
    msg.roomname[sizeof(msg.roomname) - 1] = '\0';

    fprintf(connection->out, "%s\n", toJSON(&msg));
    fflush(connection->out);

}

void handleJoinedRoom(const char *username, const char *roomname){
    printf("%s enter the room %s\n", username, roomname );
}

void handleRoomText(const char *username, const char *roomname, const char *text){
    printf("%s sent to  room %s : %s\n", username, roomname, text);

}

void handleRoomUserList(GHashTable *connections, const char *roomname) {

    if (g_hash_table_size(connections) == 0) {
        printf("No hay usuarios conectados en SALA %s. Eres tú y la soledad :( \n", roomname);
        return;
    }

    GHashTableIter iter;
    gpointer key, value;
    g_hash_table_iter_init(&iter, connections);
    while (g_hash_table_iter_next(&iter, &key, &value)) {
        printf("Username: %s, Status: %s\n", (char *)key, (char *)value);
    }
    g_hash_table_destroy(connections);

}

void handleLeftRoom(const char *username, const char *roomname ){
    printf(" %s leave the %s :( \n", username, roomname);
}

void handleReciveMessage(char* buffer, struct Connection* conection) {

    printf("ESTO VIENE DEL SERVIDOR: %s \n", buffer);
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
            handleNewStatus(message.username, message.status);
            break;
        
        case USER_LIST:
            handleUserList(message.connections);
            break;
        
        case RESPONSE:
            handleServerResponse(message);
            break;

        case INVITATION:
            handleInvitation(message.username, message.roomname, conection);
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
            // handleUnknownMessage();
            break;
    }
}

void *receiveMessageFromServer(void *arg) {
    struct Connection* connection = (struct Connection *)arg;
    char buffer[1024];
    while (true) {
        char *result = fgets(buffer, sizeof(buffer) - 1, connection->in);
        if (result != NULL) 
            handleReciveMessage(buffer, connection);
        else 
            break;
    }   

    return NULL;
}


int main(int argc, char* argv[]) {

    if (argc != 3) {
        fprintf(stderr, "Usage: %s <IP> <Port>\n", argv[0]);
        return EXIT_FAILURE;
    }

    struct Connection* connection = connetNewClient(argv[1], atoi(argv[2]));
    createReceiveMessageThreadClient(connection);
    sentMessageToServer(connection);

    return 0;
}
