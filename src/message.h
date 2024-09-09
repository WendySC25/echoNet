#ifndef MESSAGE_H
#define MESSAGE_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <glib.h>
#include "server.h"

#include "../third_party/cJSON/cJSON.h"


typedef enum {
    IDENTIFY,
    RESPONSE,
    NEW_USER,
    STATUS,
    NEW_STATUS,
    USERS,
    USER_LIST,
    TEXT,
    TEXT_FROM,
    PUBLIC_TEXT,
    PUBLIC_TEXT_FROM,
    NEW_ROOM,
    INVITE,
    INVITATION,
    JOIN_ROOM,
    JOINED_ROOM,
    ROOM_USERS,
    ROOM_USER_LIST,
    ROOM_TEXT,
    ROOM_TEXT_FROM,
    LEAVE_ROOM,
    LEFT_ROOM,
    DISCONNECT,
    DISCONNECTED,
    INVALID
} Type;

typedef enum {
    OP_IDENTIFY,
    OP_TEXT,
    OP_NEW_ROOM,
    OP_INVITE,
    OP_JOIN_ROOM,
    OP_ROOM_USERS,
    OP_ROOM_TEXT,
    OP_LEAVE_ROOM,
    OP_DISCONNECT,
    OP_INVALID
} Operation;

typedef enum {
    RE_SUCCESS,
    RE_USER_ALREADY_EXISTS,
    RE_NO_SUCH_USER,
    RE_ROOM_ALREADY_EXISTS,
    RE_NO_SUCH_ROOM,
    RE_NOT_INVITED,
    RE_NOT_JOINED,
    RE_NOT_IDENTIFIED,
    RE_INVALID
} Result;


typedef struct {
    Type  type;
    Operation    operation;
    Result       result;
    char         extra[256];
    char         username[9];
    char         status[6];
    char         roomname[17];
    char         text[256];

    char **usernamesInvitation; 

    GHashTable *connections;
    GHashTable *chat_rooms;

} Message;

const char* operationToString(Operation operation);
const char* resultToString(Result result);
Operation   getOperation(const char* str);
Result      getResult(const char* str);
char*       toJSON(Message* message);
Message     getMessage(char* jsonString);
Message     parseInput(const char *input) 

#endif 
