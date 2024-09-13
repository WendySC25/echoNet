#ifndef PROTOCOL_H
#define PROTOCOL_H

#include "message.h"
#include "server.h"

void identifyUser(struct Server* server, Message* message,  struct Connection* connection);
void changeStatus(struct Server* server, Message* message,  struct Connection* connection);
void listUsers(struct Server* server,  Message* message,  struct Connection* connection);
void sendPrivateMessage(struct Server* server, Message* message,  struct Connection* connection);
void sendPublicMessage(struct Server* server, Message* message,  struct Connection* connection);
void createRoom(struct Server* server,  Message* message,  struct Connection* connection);
void inviteToRoom(struct Server* server,  Message* message,  struct Connection* connection);
void joinRoom(struct Server* server,  Message* message,  struct Connection* connection);
void listRoomUsers(struct Server* server,  Message* message,  struct Connection* connection);
void sendRoomMessage(struct Server* server, Message* message,  struct Connection* connection);
void leaveRoom(struct Server* server,Message* message,  struct Connection* connection);
void disconnectUser(struct Server* server, Message* message,  struct Connection* connection);
void sendRoomMessageToAll(GHashTable* room_members, Message* message, struct Server* server);
void imprimeCONTROL(char* message);

#endif
