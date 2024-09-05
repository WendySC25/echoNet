#include "message.h"


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

char* toJSON(Message* message) {
    cJSON *json = cJSON_CreateObject();
    switch (message->type) {
        case IDENTIFY:
            cJSON_AddStringToObject(json, "type",     "IDENTIFY");
            cJSON_AddStringToObject(json, "username", message->username);
            break;
        
        case NEW_USER:
            cJSON_AddStringToObject(json, "type",     "NEW_USER");
            cJSON_AddStringToObject(json, "username", message->username);
            break;

        case RESPONSE:
            cJSON_AddStringToObject(json, "type",      "RESPONSE");
            cJSON_AddStringToObject(json, "operation", operationToString(message->operation));
            cJSON_AddStringToObject(json, "result",    resultToString(message->result));
            cJSON_AddStringToObject(json, "extra",     message->extra);
            break;

        default:
            cJSON_AddStringToObject(json, "type",      "INVALID");
            cJSON_AddStringToObject(json, "operation", "INVALID");
            cJSON_AddStringToObject(json, "result",    "INVALID");
            break;
    }

    char *jsonString = cJSON_PrintUnformatted(json);
    cJSON_Delete(json);
    return jsonString;
}

Message getMessage(char* jsonString) {
    Message message;
    cJSON *json = cJSON_Parse(jsonString);

    if (!json) {
        message.type = INVALID;
        return message;
    }

    const cJSON *type = cJSON_GetObjectItemCaseSensitive(json, "type");
    if (cJSON_IsString(type) && (type->valuestring != NULL)) {
        if (strcmp(type->valuestring, "RESPONSE") == 0) {
            message.type = RESPONSE;
            const cJSON *operation = cJSON_GetObjectItemCaseSensitive(json, "operation");
            const cJSON *result    = cJSON_GetObjectItemCaseSensitive(json, "result");
            const cJSON *extra     = cJSON_GetObjectItemCaseSensitive(json,  "extra");

            if (cJSON_IsString(operation)) 
                message.operation = getOperation(operation->valuestring);
            
            if (cJSON_IsString(result)) 
                message.result = getResult(result->valuestring);

            if (cJSON_IsString(extra)) 
                strncpy(message.extra, extra->valuestring, sizeof(message.extra)-1);

        } else if (strcmp(type->valuestring, "IDENTIFY") == 0) {
            message.type = IDENTIFY;
            const cJSON *username    = cJSON_GetObjectItemCaseSensitive(json, "username");

            if (cJSON_IsString(username)) 
                strcpy(message.username, username->valuestring);
        }
    }

    cJSON_Delete(json);
    return message;
}
