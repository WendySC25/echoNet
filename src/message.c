#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "/Users/wendysc/Desktop/Semestre3/MyP/echoNet/third_party/cJSON/cJSON.h"

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
    JOIN_ROOM,
    JOINED_ROOM,
    ROOM_USERS,
    ROOM_USER_LIST,
    ROOM_TEXT,
    ROOM_TEXT_FROM,
    LEAVE_ROOM,
    LEFT_ROOM,
    DISCONNECT,

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

} Message;


const char* operationToString(Operation operation) {
    switch(operation) {
        case OP_IDENTIFY:      return "IDENTIFY";
        case OP_TEXT:          return "TEXT";
        case OP_NEW_ROOM:      return "NEW_ROOM";
        case OP_INVITE:        return "INVITE";
        case OP_JOIN_ROOM:     return "JOIN_ROOM";
        case OP_ROOM_USERS:    return "ROOM_USERS";
        case OP_ROOM_TEXT:     return "ROOM_TEXT";
        case OP_LEAVE_ROOM:    return "LEAVE_ROOM";
        case OP_DISCONNECT:    return "DISCONNECT";
        default:            return "INVALID";
    }
}

const char* resultToString(Result result) {
    switch(result) {
        case RE_SUCCESS:               return "SUCCESS";
        case RE_USER_ALREADY_EXISTS:   return "USER_ALREADY_EXISTS";
        case RE_NO_SUCH_USER:          return "NO_SUCH_USER";
        case RE_ROOM_ALREADY_EXISTS:   return "ROOM_ALREADY_EXISTS";
        case RE_NO_SUCH_ROOM:          return "NO_SUCH_ROOM";
        case RE_NOT_INVITED:           return "NOT_INVITED";
        case RE_NOT_JOINED:            return "NOT_JOINED";
        case RE_NOT_IDENTIFIED:        return "NOT_IDENTIFIED";
        default:                    return "INVALID";
    }
}

Operation getOperation(const char* str) {
    if (strcmp(str, "IDENTIFY") == 0) return OP_IDENTIFY;
    if (strcmp(str, "TEXT") == 0) return OP_TEXT;
    if (strcmp(str, "NEW_ROOM") == 0) return OP_NEW_ROOM;
    if (strcmp(str, "INVITE") == 0) return OP_INVITE;
    if (strcmp(str, "JOIN_ROOM") == 0) return OP_JOIN_ROOM;
    if (strcmp(str, "ROOM_USERS") == 0) return OP_ROOM_USERS;
    if (strcmp(str, "ROOM_TEXT") == 0) return OP_ROOM_TEXT;
    if (strcmp(str, "LEAVE_ROOM") == 0) return OP_LEAVE_ROOM;
    if (strcmp(str, "DISCONNECT") == 0) return OP_DISCONNECT;
    return OP_INVALID;
}

Result getResult(const char* str) {
    if (strcmp(str, "SUCCESS") == 0) return RE_SUCCESS;
    if (strcmp(str, "USER_ALREADY_EXISTS") == 0) return RE_USER_ALREADY_EXISTS;
    if (strcmp(str, "NO_SUCH_USER") == 0) return RE_NO_SUCH_USER;
    if (strcmp(str, "ROOM_ALREADY_EXISTS") == 0) return RE_ROOM_ALREADY_EXISTS;
    if (strcmp(str, "NO_SUCH_ROOM") == 0) return RE_NO_SUCH_ROOM;
    if (strcmp(str, "NOT_INVITED") == 0) return RE_NOT_INVITED;
    if (strcmp(str, "NOT_JOINED") == 0) return RE_NOT_JOINED;
    if (strcmp(str, "NOT_IDENTIFIED") == 0) return RE_NOT_IDENTIFIED;
    return RE_INVALID;
}
