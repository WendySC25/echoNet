#ifndef MESSAGE_H
#define MESSAGE_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <glib.h>

#include "server.h"
#include "user.h"
#include "../third_party/cJSON/cJSON.h"

/**
 * @brief Enumeration of message types.
 */
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

/**
 * @brief Enumeration of message operations.
 */
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

/**
 * @brief Enumeration of result codes for responses.
 */
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

/**
 * @brief Structure representing a message.
 */
typedef struct {
    Type       type;
    Operation  operation;
    Result     result;
    UserStatus status;
    char       extra[256];
    char       username[9];
    char       roomname[17];
    char       text[256];

    GArray*    usernamesInvitation;

    GHashTable *connections;
    GHashTable *chat_rooms;
} Message;

/**
 * @brief Converts an operation to a string representation.
 * @param operation The operation to convert.
 * @return String representation of the operation.
 */
const char* operationToString(Operation operation);

/**
 * @brief Converts a result code to a string representation.
 * @param result The result code to convert.
 * @return String representation of the result.
 */
const char* resultToString(Result result);

/**
 * @brief Converts a string to an operation enum.
 * @param str The string representing the operation.
 * @return The corresponding Operation enum.
 */
Operation getOperation(const char* str);

/**
 * @brief Converts a string to a result enum.
 * @param str The string representing the result.
 * @return The corresponding Result enum.
 */
Result getResult(const char* str);

/**
 * @brief Converts a message to a JSON string.
 * @param message Pointer to the Message structure.
 * @return JSON string representing the message.
 */
char* toJSON(Message* message);

/**
 * @brief Parses a JSON string into a Message structure.
 * @param jsonString JSON string representing a message.
 * @return Parsed Message structure.
 */
Message getMessage(char* jsonString);

/**
 * @brief Parses input string to create a Message structure.
 * @param input Input string.
 * @return Parsed Message structure.
 */
Message parseInput(const char *input);

#endif // MESSAGE_H
