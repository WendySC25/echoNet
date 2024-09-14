#include "clientInterface.h"
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

#include <unistd.h>
#include <sys/ioctl.h>


void printCentered(const char *text) {
    int start_col = 40;
    
    struct winsize ws;
    if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &ws) == -1) {
        perror("ioctl");
        exit(EXIT_FAILURE);
    }

    int terminal_width = ws.ws_col;
    int text_length = strlen(text);
    
    int max_start_col = terminal_width - text_length;
    if (start_col > max_start_col) {
        start_col = max_start_col;
    }

    printf("%*s%s\n", start_col, "", text);
}

void handleNewUser(const char *username) {
    char message[1024];
    snprintf(message, sizeof(message), BOLD GREEN "%s has entered the chat." RESET, username);
    printCentered(message);
}

void handleNewStatus(const char *username, const char *status) {
    char message[1024];
    snprintf(message, sizeof(message), BOLD "%s has changed status to: %s." RESET, username, status);
    printCentered(message);
}

void handleServerResponse(Message msg, struct Connection* connection) {

    char message[1024];

    switch (msg.operation) {

    case OP_IDENTIFY:
        if(msg.result == RE_SUCCESS) {
            connection->user = newUser(msg.extra);   
            snprintf(message, sizeof(message), GREEN BOLD "Success! Username registered: %s" RESET, connection->user->username);

        } else if (msg.result == RE_USER_ALREADY_EXISTS) {
            snprintf(message, sizeof(message), RED BOLD "Sorry! Another username is already registered with name %s. Try another." RESET, msg.extra);
        }
        break;
    
    case OP_TEXT:
        snprintf(message, sizeof(message), RED BOLD "No such user: %s" RESET, msg.extra);
        break;
    
    case OP_NEW_ROOM:
        if(msg.result == RE_SUCCESS) 
            snprintf(message, sizeof(message), GREEN BOLD "Success! You have a new room: %s." RESET, msg.extra);
        else if (msg.result == RE_ROOM_ALREADY_EXISTS)
            snprintf(message, sizeof(message), RED BOLD "Sorry! Another username already registered a room: %s. Try another." RESET, msg.extra);
        break;
    
    case OP_INVITE:
        if(msg.result ==  RE_NO_SUCH_ROOM) 
            snprintf(message, sizeof(message), RED BOLD "Ups! There is no room: %s" RESET, msg.extra);
        else if(msg.result == RE_NO_SUCH_USER)  
            snprintf(message, sizeof(message), RED BOLD "Ups! There is no user: %s" RESET, msg.extra);
        break;

    case OP_JOIN_ROOM:
        if(msg.result ==  RE_NO_SUCH_ROOM) 
            snprintf(message, sizeof(message), RED BOLD "Ups! There is no room: %s" RESET, msg.extra);
        else if(msg.result == RE_NOT_INVITED) 
            snprintf(message, sizeof(message), RED BOLD "Sorry! You were not invited to room: %s" RESET, msg.extra);
        else if (msg.result == RE_SUCCESS) 
            snprintf(message, sizeof(message), GREEN BOLD "Great! Welcome to room: %s" RESET, msg.extra);
        break;
    
    case OP_ROOM_USERS:
        if(msg.result ==  RE_NO_SUCH_ROOM) 
            snprintf(message, sizeof(message), RED BOLD "Ups! There is no room: %s" RESET, msg.extra);
        else if(msg.result == RE_NOT_JOINED) 
            snprintf(message, sizeof(message), RED BOLD "Sorry! You have not joined the room: %s" RESET, msg.extra);
        break;
    
    case OP_ROOM_TEXT:
        if(msg.result ==  RE_NO_SUCH_ROOM) 
            snprintf(message, sizeof(message), RED BOLD "Ups! There is no room: %s" RESET, msg.extra);
        else if(msg.result == RE_NOT_JOINED) 
            snprintf(message, sizeof(message), RED BOLD "Sorry! You have not joined the room: %s" RESET, msg.extra);
        break;

    case OP_LEAVE_ROOM:
        if(msg.result ==  RE_NO_SUCH_ROOM) 
            snprintf(message, sizeof(message), RED BOLD "Ups! There is no room: %s" RESET, msg.extra);
        else if(msg.result == RE_NOT_JOINED) 
            snprintf(message, sizeof(message), RED BOLD "Sorry! You have not joined the room: %s" RESET, msg.extra);
        break;

    default:

        break;
    }

    printCentered(message);

}

void handlePrivateMessage(const char *username, const char *text) {
    char message[1024];
    snprintf(message, sizeof(message), ITALIC " %s whispered to you: %s" RESET, username, text);
    printCentered(message);
}

void handlePublicMessage(const char *username, const char *text) {
    char message[1024];
    snprintf(message, sizeof(message), BOLD " %s (global): %s" RESET, username, text);
    printCentered(message);
}

void handleDisconnection(const char *username) {
    char message[1024];
    snprintf(message, sizeof(message), BOLD RED " %s has left the chat :(" RESET, username);
    printCentered(message);
}

void handleUserList(GHashTable *connections) {
    if (g_hash_table_size(connections) == 0) {
        printCentered("No users connected. It's just you and solitude :(");
        return;
    }

    GHashTableIter iter;
    gpointer key, value;
    g_hash_table_iter_init(&iter, connections);
    while (g_hash_table_iter_next(&iter, &key, &value)) {
        char message[1024];
        snprintf(message, sizeof(message), "Username: %s, Status: %s\n", (char *)key, (char *)value);
        printCentered(message);
    }
    g_hash_table_destroy(connections);
}

void handleInvitation(const char *username, const char *roomname) {
    char message[1024];
    snprintf(message, sizeof(message), BOLD " %s invited you to room %s" RESET, username, roomname);
    printCentered(message);
}

void handleJoinedRoom(const char *username, const char *roomname) {
    char message[1024];
    snprintf(message, sizeof(message), BOLD GREEN "%s has entered room %s" RESET, username, roomname);
    printCentered(message);
}

void handleRoomText(const char *username, const char *roomname, const char *text) {
    char message[1024];
    snprintf(message, sizeof(message), BOLD "%s (room %s): %s" RESET, username, roomname, text);
    printCentered(message);
}

void handleRoomUserList(GHashTable *connections, const char *roomname) {
    if (g_hash_table_size(connections) == 0) {
        char message[1024];
        snprintf(message, sizeof(message), "No users connected in ROOM %s. It's just you and solitude :(", roomname);
        printCentered(message);
        return;
    }

    GHashTableIter iter;
    gpointer key, value;
    g_hash_table_iter_init(&iter, connections);
    while (g_hash_table_iter_next(&iter, &key, &value)) {
        char message[1024];
        snprintf(message, sizeof(message), "Username: %s, Status: %s\n", (char *)key, (char *)value);
        printCentered(message);
    }
    g_hash_table_destroy(connections);
}

void handleLeftRoom(const char *username, const char *roomname) {
    char message[1024];
    snprintf(message, sizeof(message), BOLD RED " %s has left the room %s :(" RESET, username, roomname);
    printCentered(message);
}
