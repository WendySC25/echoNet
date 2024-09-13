#ifndef CLIENTINTERFACE_H
#define CLIENTINTERFACE_H

#include <glib.h>
#include "server.h" 
#include "message.h"

#define CIAN  "\x1b[106m"
#define RESET "\x1b[0m"
#define BOLD  "\x1b[1m"
#define ITALIC "\x1b[3m"
#define GREEN "\x1b[32m"     
#define RED "\x1b[31m"  // Rojo


void printCentered(const char *text);
void handleNewUser(const char *username);
void handleServerResponse(Message msg, struct Connection* connection);
void handlePrivateMessage(const char *username, const char *text);
void handlePublicMessage(const char *username, const char *text);
void handleNewStatus(const char *username, const char *status);
void handleUserList(GHashTable *connections);
void handleInvitation(const char *username, const char *roomname, struct Connection* connection);
void handleJoinedRoom(const char *username, const char *roomname);
void handleRoomText(const char *username, const char *roomname, const char *text);
void handleRoomUserList(GHashTable *connections, const char *roomname);
void handleLeftRoom(const char *username, const char *roomname );
void handleDisconnection(const char *username);

#endif
