#include "message.h"
#include "server.h"
#include <glib.h>
#include <stdio.h>

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

        case STATUS:
            cJSON_AddStringToObject(json, "type", "STATUS");
            cJSON_AddStringToObject(json, "status", message->status);
            break;
        
        case NEW_STATUS:
            cJSON_AddStringToObject(json, "type", "NEW_STATUS");
            cJSON_AddStringToObject(json, "username", message->username);
            cJSON_AddStringToObject(json, "status", message->status);
            break;


        case USERS:
            cJSON_AddStringToObject(json, "type", "USERS");
            break;
        
        case USER_LIST:
            cJSON_AddStringToObject(json, "type", "USER_LIST");

            cJSON *users = cJSON_AddObjectToObject(json, "users");

            GHashTableIter iter;
            gpointer key, value;
            g_hash_table_iter_init(&iter, message->connections);
            while (g_hash_table_iter_next(&iter, &key, &value)) {
                struct Connection *conn = (struct Connection *)value;
                if (conn && conn->user && conn->user->username) 
                    cJSON_AddStringToObject(users, conn->user->username, conn->user->status);
            }

            break;


        case TEXT:
            cJSON_AddStringToObject(json, "type", "TEXT");
            cJSON_AddStringToObject(json, "username", message->username);
            cJSON_AddStringToObject(json, "text", message->text);
            break;
        
        case TEXT_FROM:
            cJSON_AddStringToObject(json, "type", "TEXT_FROM");
            cJSON_AddStringToObject(json, "username", message->username);
            cJSON_AddStringToObject(json, "text", message->text);
            break;

        case PUBLIC_TEXT:
            cJSON_AddStringToObject(json, "type", "PUBLIC_TEXT");
            cJSON_AddStringToObject(json, "text", message->text);
            break;
        
        case PUBLIC_TEXT_FROM:
            cJSON_AddStringToObject(json, "type", "PUBLIC_TEXT_FROM");
            cJSON_AddStringToObject(json, "username", message->username);
            cJSON_AddStringToObject(json, "text", message->text);
            break;

        case NEW_ROOM:
            cJSON_AddStringToObject(json, "type", "NEW_ROOM");
            cJSON_AddStringToObject(json, "roomname", message->roomname);
            
            break;

        case INVITE:
            cJSON_AddStringToObject(json, "type", "INVITE");
            // cJSON_AddStringToObject(json, "roomname", message->roomname);
            // cJSON *users = cJSON_AddArrayToObject(json, "usernames");
            // for (int i = 0; message->usernames[i] != NULL; i++) 
            //         cJSON_AddItemToArray(users, cJSON_CreateString(message->usernamesInvitation[i]));
            break;
        
        case INVITATION:
            cJSON_AddStringToObject(json, "type", "INVITATION");
            cJSON_AddStringToObject(json, "username", message->username);
            cJSON_AddStringToObject(json, "roomname", message->roomname);
            break;


        case JOIN_ROOM:
            cJSON_AddStringToObject(json, "type", "JOIN_ROOM");
            cJSON_AddStringToObject(json, "roomname", message->roomname);
            break;

        case JOINED_ROOM:
            cJSON_AddStringToObject(json, "type", "JOINED_ROOM");
            cJSON_AddStringToObject(json, "roomname", message->roomname);
            cJSON_AddStringToObject(json, "username", message->username);

        case ROOM_USERS:
            cJSON_AddStringToObject(json, "type", "ROOM_USERS");
            cJSON_AddStringToObject(json, "roomname", message->roomname);
            break;
        
        case ROOM_USER_LIST:
            cJSON_AddStringToObject(json, "type", "ROOM_USER_LIST");
            // cJSON *users = cJSON_AddObjectToObject(json, "users");

            // GHashTableIter iter;
            // gpointer key, value;
            // g_hash_table_iter_init(&iter, message->chat_rooms->clients);
            // while (g_hash_table_iter_next(&iter, &key, &value)) {
            //     Connection *conn = (Connection *)value;
            //     if (conn && conn->user && conn->user->username) 
            //         cJSON_AddStringToObject(users, conn->user->username, conn->user->status);
            // }
            break;


        case ROOM_TEXT:
            cJSON_AddStringToObject(json, "type", "ROOM_TEXT");
            cJSON_AddStringToObject(json, "roomname", message->roomname);
            cJSON_AddStringToObject(json, "text", message->text);
            break;
        
        case ROOM_TEXT_FROM:
            cJSON_AddStringToObject(json, "type", "ROOM_TEXT_FROM");
            cJSON_AddStringToObject(json, "roomname", message->roomname);
            cJSON_AddStringToObject(json, "username", message->username);
            cJSON_AddStringToObject(json, "text", message->text);
            break;

        case LEAVE_ROOM:
            cJSON_AddStringToObject(json, "type", "LEAVE_ROOM");
            cJSON_AddStringToObject(json, "roomname", message->roomname);
            break;

        case LEFT_ROOM:
            cJSON_AddStringToObject(json, "type", "LEFT_ROOM");
            cJSON_AddStringToObject(json, "roomname", message->roomname);
            cJSON_AddStringToObject(json, "username", message->username);
            break;
            

        case DISCONNECT:
            cJSON_AddStringToObject(json, "type", "DISCONNECT");
            break;

        case DISCONNECTED:
            cJSON_AddStringToObject(json, "type", "DISCONNECTED");
            cJSON_AddStringToObject(json, "username", message->username);
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
    if (!cJSON_IsString(type) || (type->valuestring == NULL)) {
        message.type = INVALID;
        cJSON_Delete(json);
        return message;
    }

    if (strcmp(type->valuestring, "RESPONSE") == 0) {
        message.type = RESPONSE;
        const cJSON *operation = cJSON_GetObjectItemCaseSensitive(json, "operation");
        const cJSON *result    = cJSON_GetObjectItemCaseSensitive(json, "result");
        const cJSON *extra     = cJSON_GetObjectItemCaseSensitive(json, "extra");

        if (cJSON_IsString(operation)) 
            message.operation = getOperation(operation->valuestring);
        
        if (cJSON_IsString(result)) 
            message.result = getResult(result->valuestring);

        if (cJSON_IsString(extra)) 
            strncpy(message.extra, extra->valuestring, sizeof(message.extra)-1);

    } else if (strcmp(type->valuestring, "IDENTIFY") == 0) {
        message.type = IDENTIFY;
        const cJSON *username = cJSON_GetObjectItemCaseSensitive(json, "username");

        if (cJSON_IsString(username)) 
            strncpy(message.username, username->valuestring, sizeof(message.username)-1);

    } else if (strcmp(type->valuestring, "NEW_USER") == 0) {
        message.type = NEW_USER;
        const cJSON *username = cJSON_GetObjectItemCaseSensitive(json, "username");

        if (cJSON_IsString(username)) 
            strncpy(message.username, username->valuestring, sizeof(message.username)-1);

    } else if (strcmp(type->valuestring, "STATUS") == 0) {
        message.type = STATUS;
        const cJSON *status = cJSON_GetObjectItemCaseSensitive(json, "status");

        if (cJSON_IsString(status)) 
            strncpy(message.status, status->valuestring, sizeof(message.status)-1);

    } else if (strcmp(type->valuestring, "NEW_STATUS") == 0) {
        message.type = NEW_STATUS;
        const cJSON *username = cJSON_GetObjectItemCaseSensitive(json, "username");
        const cJSON *status = cJSON_GetObjectItemCaseSensitive(json, "status");

        if (cJSON_IsString(username)) 
            strncpy(message.username, username->valuestring, sizeof(message.username)-1);
        
        if (cJSON_IsString(status)) 
            strncpy(message.status, status->valuestring, sizeof(message.status)-1);

    } else if (strcmp(type->valuestring, "USERS") == 0) {
        message.type = USERS;

    } else if (strcmp(type->valuestring, "USER_LIST") == 0) {
        
        message.type = USER_LIST;

        cJSON *users = cJSON_GetObjectItemCaseSensitive(json, "users");
        if (!cJSON_IsObject(users)) {
            fprintf(stderr, "Error: 'users' is not an object\n");
            cJSON_Delete(json);
            message.type = INVALID;
        }

        GHashTable *dict = g_hash_table_new_full(g_str_hash, g_str_equal, g_free, g_free);

        cJSON *user;
        cJSON_ArrayForEach(user, users) {
            if (cJSON_IsString(user)) {
                const char *username = user->string;
                const char *status = cJSON_GetStringValue(user);
                g_hash_table_insert(dict, g_strdup(username), g_strdup(status));
            }
        }
        message.connections = dict;

    } else if (strcmp(type->valuestring, "TEXT") == 0) {
        message.type = TEXT;
        const cJSON *username = cJSON_GetObjectItemCaseSensitive(json, "username");
        const cJSON *text = cJSON_GetObjectItemCaseSensitive(json, "text");

        if (cJSON_IsString(username)) 
            strncpy(message.username, username->valuestring, sizeof(message.username)-1);
        
        if (cJSON_IsString(text)) 
            strncpy(message.text, text->valuestring, sizeof(message.text)-1);

    } else if (strcmp(type->valuestring, "TEXT_FROM") == 0) {
        message.type = TEXT_FROM;
        const cJSON *username = cJSON_GetObjectItemCaseSensitive(json, "username");
        const cJSON *text = cJSON_GetObjectItemCaseSensitive(json, "text");

        if (cJSON_IsString(username)) 
            strncpy(message.username, username->valuestring, sizeof(message.username)-1);
        
        if (cJSON_IsString(text)) 
            strncpy(message.text, text->valuestring, sizeof(message.text)-1);

    } else if (strcmp(type->valuestring, "PUBLIC_TEXT") == 0) {
        message.type = PUBLIC_TEXT;
        const cJSON *username = cJSON_GetObjectItemCaseSensitive(json, "username");
        const cJSON *text = cJSON_GetObjectItemCaseSensitive(json, "text");

        if (cJSON_IsString(username)) 
            strncpy(message.username, username->valuestring, sizeof(message.username)-1);
        
        if (cJSON_IsString(text)) 
            strncpy(message.text, text->valuestring, sizeof(message.text)-1);

    } else if (strcmp(type->valuestring, "PUBLIC_TEXT_FROM") == 0) {
        message.type = PUBLIC_TEXT_FROM;
        const cJSON *username = cJSON_GetObjectItemCaseSensitive(json, "username");
        const cJSON *text = cJSON_GetObjectItemCaseSensitive(json, "text");

        if (cJSON_IsString(username)) 
            strncpy(message.username, username->valuestring, sizeof(message.username)-1);
        
        if (cJSON_IsString(text)) 
            strncpy(message.text, text->valuestring, sizeof(message.text)-1);

    } else if (strcmp(type->valuestring, "NEW_ROOM") == 0) {
        message.type = NEW_ROOM;
        const cJSON *roomname = cJSON_GetObjectItemCaseSensitive(json, "roomname");
        if (cJSON_IsString(roomname)) 
            strncpy(message.roomname, roomname->valuestring, sizeof(message.roomname) - 1);

    } else if (strcmp(type->valuestring, "INVITE") == 0) {
        message.type = INVITE;
        const cJSON *roomname = cJSON_GetObjectItemCaseSensitive(json, "roomname");
        const cJSON *usernames = cJSON_GetObjectItemCaseSensitive(json, "usernames");

        if (cJSON_IsString(roomname)) 
            strncpy(message.roomname, roomname->valuestring, sizeof(message.roomname) - 1);
        
        if (cJSON_IsArray(usernames)) {
            // Aquí se debería inicializar un arreglo o lista para los nombres de usuario
            cJSON *username;
            cJSON_ArrayForEach(username, usernames) {
                if (cJSON_IsString(username)) {
                    // Aquí se debería agregar el nombre de usuario al arreglo o lista
                }
            }
        }

    } else if (strcmp(type->valuestring, "INVITATION") == 0) {
        message.type = INVITATION;
        const cJSON *username = cJSON_GetObjectItemCaseSensitive(json, "username");
        const cJSON *roomname = cJSON_GetObjectItemCaseSensitive(json, "roomname");

        if (cJSON_IsString(username)) 
            strncpy(message.username, username->valuestring, sizeof(message.username) - 1);
        
        if (cJSON_IsString(roomname)) 
            strncpy(message.roomname, roomname->valuestring, sizeof(message.roomname) - 1);

    } else if (strcmp(type->valuestring, "JOIN_ROOM") == 0) {
        message.type = JOIN_ROOM;
        const cJSON *roomname = cJSON_GetObjectItemCaseSensitive(json, "roomname");
        if (cJSON_IsString(roomname)) 
            strncpy(message.roomname, roomname->valuestring, sizeof(message.roomname) - 1);

    } else if (strcmp(type->valuestring, "JOINED_ROOM") == 0) {
        message.type = JOINED_ROOM;
        const cJSON *roomname = cJSON_GetObjectItemCaseSensitive(json, "roomname");
        const cJSON *username = cJSON_GetObjectItemCaseSensitive(json, "username");

        if (cJSON_IsString(roomname)) 
            strncpy(message.roomname, roomname->valuestring, sizeof(message.roomname)-1);
        
        if (cJSON_IsString(username)) 
            strncpy(message.username, username->valuestring, sizeof(message.username)-1);

    } else if (strcmp(type->valuestring, "ROOM_USERS") == 0) {
        message.type = ROOM_USERS;
        const cJSON *roomname = cJSON_GetObjectItemCaseSensitive(json, "roomname");
        if (cJSON_IsString(roomname)) 
            strncpy(message.roomname, roomname->valuestring, sizeof(message.roomname) - 1);

    } else if (strcmp(type->valuestring, "ROOM_USER_LIST") == 0) {
        message.type = ROOM_USER_LIST;
        // Similar al caso "USER_LIST", se podría inicializar un hash table para almacenar los usuarios y sus estados.

        const cJSON *roomname = cJSON_GetObjectItemCaseSensitive(json, "roomname");
        const cJSON *usernames = cJSON_GetObjectItemCaseSensitive(json, "usernames");

        if (cJSON_IsString(roomname)) 
            strncpy(message.roomname, roomname->valuestring, sizeof(message.roomname) - 1);
        
        if (cJSON_IsArray(usernames)) {
            // Aquí se debería inicializar un arreglo o lista para los nombres de usuario
            cJSON *username;
            cJSON_ArrayForEach(username, usernames) {
                if (cJSON_IsString(username)) {
                    // Aquí se debería agregar el nombre de usuario al arreglo o lista
                }
            }
        }

    } else if (strcmp(type->valuestring, "ROOM_TEXT") == 0) {
        message.type = ROOM_TEXT;
        const cJSON *roomname = cJSON_GetObjectItemCaseSensitive(json, "roomname");
        const cJSON *text = cJSON_GetObjectItemCaseSensitive(json, "text");

        if (cJSON_IsString(roomname)) 
            strncpy(message.roomname, roomname->valuestring, sizeof(message.roomname) - 1);
        
        if (cJSON_IsString(text)) 
            strncpy(message.text, text->valuestring, sizeof(message.text) - 1);

    } else if (strcmp(type->valuestring, "ROOM_TEXT_FROM") == 0) {
        message.type = ROOM_TEXT_FROM;
        const cJSON *roomname = cJSON_GetObjectItemCaseSensitive(json, "roomname");
        const cJSON *username = cJSON_GetObjectItemCaseSensitive(json, "username");
        const cJSON *text = cJSON_GetObjectItemCaseSensitive(json, "text");

        if (cJSON_IsString(roomname)) 
            strncpy(message.roomname, roomname->valuestring, sizeof(message.roomname)-1);
        
        if (cJSON_IsString(username)) 
            strncpy(message.username, username->valuestring, sizeof(message.username)-1);
        
        if (cJSON_IsString(text)) 
            strncpy(message.text, text->valuestring, sizeof(message.text)-1);

    } else if (strcmp(type->valuestring, "LEAVE_ROOM") == 0) {
        message.type = LEAVE_ROOM;
        const cJSON *roomname = cJSON_GetObjectItemCaseSensitive(json, "roomname");
        const cJSON *username = cJSON_GetObjectItemCaseSensitive(json, "username");

         if (cJSON_IsString(roomname)) 
            strncpy(message.roomname, roomname->valuestring, sizeof(message.roomname)-1);
        
        if (cJSON_IsString(username)) 
            strncpy(message.username, username->valuestring, sizeof(message.username)-1);


    } else if (strcmp(type->valuestring, "LEFT_ROOM") == 0) {
        message.type = LEFT_ROOM;
        const cJSON *roomname = cJSON_GetObjectItemCaseSensitive(json, "roomname");
        const cJSON *username = cJSON_GetObjectItemCaseSensitive(json, "username");

        if (cJSON_IsString(roomname)) 
            strncpy(message.roomname, roomname->valuestring, sizeof(message.roomname)-1);
        
        if (cJSON_IsString(username)) 
            strncpy(message.username, username->valuestring, sizeof(message.username)-1);

    } else if (strcmp(type->valuestring, "DISCONNECT") == 0) {
        message.type = DISCONNECT;

    } else if (strcmp(type->valuestring, "DISCONNECTED") == 0) {
        message.type = DISCONNECTED;
        const cJSON *username = cJSON_GetObjectItemCaseSensitive(json, "username");

        if (cJSON_IsString(username)) 
            strncpy(message.username, username->valuestring, sizeof(message.username)-1);

    } else {
        message.type = INVALID;
    }

    cJSON_Delete(json);
    return message;
}

Message parseInput(const char *input) {
    Message msg;
    memset(&msg, 0, sizeof(msg));

    if (input == NULL) {
        msg.type = INVALID;
        return msg;
    }

    gchar **parts = g_strsplit_set(input, " \t", -1);
    if (parts == NULL) {
        msg.type = INVALID;
        return msg;
    }

    // Comando \whisper
    if (g_strcmp0(parts[0], "\\whisper") == 0) {
        msg.type = TEXT;
        
        if (parts[1] != NULL) {
            // El nombre de usuario es la segunda parte
            g_strlcpy(msg.username, parts[1], sizeof(msg.username));

            // El mensaje es todo lo que sigue después del nombre de usuario
            gchar *message = g_strjoinv(" ", &parts[2]);
            if (message != NULL) {
                g_strlcpy(msg.text, message, sizeof(msg.text));
                g_free(message);
            } else {
                msg.type = INVALID;
            }
        } else {
            msg.type = INVALID;
        }
    } 
    // Comando \broadcast
    else if (g_strcmp0(parts[0], "\\broadcast") == 0) {
        msg.type = PUBLIC_TEXT;
        gchar *message = g_strjoinv(" ", &parts[1]);
        if (message != NULL) {
            g_strlcpy(msg.text, message, sizeof(msg.text));
            g_free(message);
        } else {
            msg.type = INVALID;
        }
    } 

    else if (g_strcmp0(parts[0], "\\getUsers") == 0) {
        msg.type = USERS;
    } 

    // Comando \setStatus
    else if (g_strcmp0(parts[0], "\\setStatus") == 0) {
        msg.type = STATUS;
        if (parts[1] != NULL) {
            g_strlcpy(msg.status, parts[1], sizeof(msg.status));
        } else {
            msg.type = INVALID;
        }
    } 

    else if (g_strcmp0(parts[0], "\\bye") == 0) {
        msg.type = DISCONNECT;
    } 
    
    else {
        msg.type = INVALID;
    }

    g_strfreev(parts);
    return msg;
}