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
    
    // Asegurarse de que el texto no se pase de la terminal
    int max_start_col = terminal_width - text_length;
    if (start_col > max_start_col) {
        start_col = max_start_col;
    }

    // Imprimir el texto comenzando en la columna deseada
    printf("%*s%s\n", start_col, "", text);
}

void handleNewUser(const char *username) {
    char message[1024];
    snprintf(message, sizeof(message), BOLD GREEN "%s has entered the chat %s" RESET, username);
    printCentered(message);
}

void handleNewStatus(const char *username, const char *status) {
    char message[1024];
    snprintf(message, sizeof(message), BOLD "%s has changed status to: %s." RESET, username, status);
    printCentered(message);
}

void handleServerResponse(Message msg, struct Connection* connection) {
    if (msg.operation == OP_IDENTIFY && msg.result == RE_SUCCESS) {
        connection->user = newUser(msg.extra);
        char message[1024];
        snprintf(message, sizeof(message), BOLD "Success! Username registered: %s" RESET, connection->user->username);
        printCentered(message);
    }

    if (msg.operation == OP_INVALID) {
        exit(EXIT_SUCCESS);
    }

    char response[1024];
    snprintf(response, sizeof(response), "%s", toJSON(&msg));
    printCentered(response);
}

void handlePrivateMessage(const char *username, const char *text) {
    char message[1024];
    snprintf(message, sizeof(message), ITALIC " %s whispered to you: %s" RESET, username, text);
    printCentered(message);
}

void handlePublicMessage(const char *username, const char *text) {
    char message[1024];
    snprintf(message, sizeof(message), BOLD " %s sent to global chat: %s" RESET, username, text);
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

void handleInvitation(const char *username, const char *roomname, struct Connection* connection) {
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
    snprintf(message, sizeof(message), BOLD "%s sent to room %s: %s" RESET, username, roomname, text);
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