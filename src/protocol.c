#include "protocol.h" 
#include "message.h"
#include <string.h>
#include "server.h"
#include <string.h>
#include "user.h"

void imprimeCONTROL(char* message){
    printf("Este mensaje será enviado: %s\n", message);
}

void handlesUnidentifiedUser(struct Server* server, Message* message,  struct Connection* connection){
     Message serverResponse = {
        .type = RESPONSE,
        .operation = OP_INVALID,
        .result = RE_NOT_IDENTIFIED
    };

    sendTo(toJSON(&serverResponse), connection);
    freeConnection(connection);
}


void identifyUser(struct Server* server, Message* message, struct Connection* connection) {

    //EVITAR QUE EL USUARIO IDENTIFICADO ÉXITOSAMENTE SE VUELVA A REGISTRAR CON OTRO NOMBRE
    if(strcmp(connection->user->username, "newuser") != 0) {
        return;
    }

    Message serverResponse = {
        .type = RESPONSE,
        .operation = OP_IDENTIFY,
    };

    strncpy(serverResponse.extra, message->username, sizeof(serverResponse.extra) - 1);
    serverResponse.extra[sizeof(serverResponse.extra) - 1] = '\0';

    struct Connection *existing_connection = (struct Connection *)g_hash_table_lookup(server->connections, message->username);
    if (existing_connection != NULL) {
        serverResponse.result = RE_USER_ALREADY_EXISTS;
        sendTo(toJSON(&serverResponse), connection);
        return;
    }

    Message notifyNewUser = {
        .type = NEW_USER,
    };

    strncpy(notifyNewUser.username, message->username, sizeof(notifyNewUser.username) - 1);
    notifyNewUser.username[sizeof(notifyNewUser.username) - 1] = '\0';

    User* user = newUser(message->username);
    connection->user = user;

    add_connection(server, message->username, connection);
    sendTo(toJSON(&serverResponse), connection);
    sendToGlobalChat(toJSON(&notifyNewUser), connection->acceptedSocketFD, server);
    

}

void changeStatus(struct Server* server, Message* message, struct Connection* connection) {
   
    struct Connection *conn = (struct Connection *)g_hash_table_lookup(server->connections, message->username);

    if (conn == NULL) {
        return;
    }

    // Actualizar el estado del usuario
    // strncpy(conn->user->status, message->status, sizeof(conn->user->status) - 1);
    // conn->user->status[sizeof(conn->user->status) - 1] = '\0';

    conn->user->status = message->status;

    Message newStatusMessage = {
        .type = NEW_STATUS,
    };
    
    strncpy(newStatusMessage.username, message->username, sizeof(newStatusMessage.username) - 1);
    newStatusMessage.username[sizeof(newStatusMessage.username) - 1] = '\0';

    newStatusMessage.status = message->status;

    // strncpy(newStatusMessage.status, message->status, sizeof(newStatusMessage.status) - 1);
    // newStatusMessage.status[sizeof(newStatusMessage.status) - 1] = '\0';

    sendToGlobalChat(toJSON(&newStatusMessage), connection->acceptedSocketFD, server);
    
}

void listUsers(struct Server* server, Message* message, struct Connection* connection) {

    Message serverResponse = {
        .type = USER_LIST,
        .connections = server->connections,
    };

    printf("%s\n", toJSON(&serverResponse));
    sendTo(toJSON(&serverResponse), connection);
}

void sendPrivateMessage(struct Server* server, Message* message, struct Connection* connection) {
   
    struct Connection *recipientConnection = (struct Connection *)g_hash_table_lookup(server->connections, message->username);

    if (recipientConnection == NULL) {

          GHashTableIter iter;
            gpointer key, value;
            g_hash_table_iter_init(&iter, server->connections);

            while (g_hash_table_iter_next(&iter, &key, &value)) {
                struct Connection *conn = (struct Connection *)value;
                printf("LISTA DE USUARIO: %s \n", conn->user->username);
            }
        Message errorResponse = {
            .type = RESPONSE,
            .operation = OP_TEXT,
            .result = RE_NO_SUCH_USER,
        };
        strncpy(errorResponse.extra, message->username, sizeof(errorResponse.extra) - 1);
         errorResponse.extra[sizeof(errorResponse.extra) - 1] = '\0';
        sendTo(toJSON(&errorResponse), connection);

        return;
    }



    Message textFromMessage = {
        .type = TEXT_FROM,
    };
    
    strncpy(textFromMessage.username, connection->user->username, sizeof(textFromMessage.username) - 1);
    textFromMessage.username[sizeof(textFromMessage.username) - 1] = '\0';

    strncpy(textFromMessage.text, message->text, sizeof(textFromMessage.text) - 1);
    textFromMessage.text[sizeof(textFromMessage.text) - 1] = '\0';


    sendTo(toJSON(&textFromMessage), recipientConnection);
}

void sendPublicMessage(struct Server* server, Message* message, struct Connection* connection) {
    Message publicTextFromMessage = {
        .type = PUBLIC_TEXT_FROM,
    };

    strncpy(publicTextFromMessage.username, message->username, sizeof(publicTextFromMessage.username) - 1);
    publicTextFromMessage.username[sizeof(publicTextFromMessage.username) - 1] = '\0';

    strncpy(publicTextFromMessage.text, message->text, sizeof(publicTextFromMessage.text) - 1);
    publicTextFromMessage.text[sizeof(publicTextFromMessage.text) - 1] = '\0';

    sendToGlobalChat(toJSON(&publicTextFromMessage), connection->acceptedSocketFD, server);

}

void createRoom(struct Server* server, Message* message, struct Connection* connection) {
    g_mutex_lock(&(server->room_mutex));
    
    // Verificar si la sala ya existe
    if (g_hash_table_contains(server->chat_rooms, message->roomname)) {
        Message response = {
            .type = RESPONSE,
            .operation = OP_NEW_ROOM,
            .result = RE_ROOM_ALREADY_EXISTS,
        };
        strncpy(response.extra, message->roomname, sizeof(response.extra) - 1);
        response.extra[sizeof(response.extra) - 1] = '\0';
        sendTo(toJSON(&response), connection);
        g_mutex_unlock(&(server->room_mutex));
        return;
    }

    // Crear nueva sala y agregar el usuario como miembro
    GHashTable* room_members = g_hash_table_new_full(g_str_hash, g_str_equal, g_free, NULL);
    g_hash_table_insert(room_members, g_strdup(connection->user->username), g_strdup("ACTIVE"));
    g_hash_table_insert(server->chat_rooms, g_strdup(message->roomname), room_members);

    Message response = {
        .type = RESPONSE,
        .operation = OP_NEW_ROOM,
        .result = RE_SUCCESS,
    };
    strncpy(response.extra, message->roomname, sizeof(response.extra) - 1);
    response.extra[sizeof(response.extra) - 1] = '\0';
    sendTo(toJSON(&response), connection);

    g_mutex_unlock(&(server->room_mutex));
}

void inviteToRoom(struct Server* server, Message* message, struct Connection* connection) {
    g_mutex_lock(&(server->room_mutex));
    
    // Verificar si la sala existe
    GHashTable* room_members = g_hash_table_lookup(server->chat_rooms, message->roomname);
    if (room_members == NULL) {
        Message response = {
            .type = RESPONSE,
            .operation = OP_INVITE,
            .result = RE_NO_SUCH_ROOM,
        };
        strncpy(response.extra, message->roomname, sizeof(response.extra) - 1);
        response.extra[sizeof(response.extra) - 1] = '\0';
        sendTo(toJSON(&response), connection);
        g_mutex_unlock(&(server->room_mutex));
        return;
    }

    // Verificar si el usuario está en la sala
    if (!g_hash_table_contains(room_members, connection->user->username)) {
        Message response = {
            .type = RESPONSE,
            .operation = OP_INVITE,
            .result = RE_NOT_INVITED,
        };
        strncpy(response.extra, message->roomname, sizeof(response.extra) - 1);
        response.extra[sizeof(response.extra) - 1] = '\0';
        sendTo(toJSON(&response), connection);
        g_mutex_unlock(&(server->room_mutex));
        return;
    }

    for (guint i = 0; i < message->usernamesInvitation->len; i++) {
    const char* invited_user = g_array_index(message->usernamesInvitation, const char*, i);
    struct Connection* invited_connection = (struct Connection*)g_hash_table_lookup(server->connections, invited_user);
    
    if (invited_connection == NULL) {
        Message response = {
            .type = RESPONSE,
            .operation = OP_INVITE,
            .result = RE_NO_SUCH_USER,
        };
        strncpy(response.extra, invited_user, sizeof(response.extra) - 1);
        response.extra[sizeof(response.extra) - 1] = '\0';
        sendTo(toJSON(&response), connection);
        continue;
    }

    if (g_hash_table_contains(room_members, invited_user)) {
        continue; // Usuario ya está en la sala o ya fue invitado
    }

    g_hash_table_insert(room_members, g_strdup(invited_user), g_strdup("INVITED"));

    // Enviar invitación al usuario
    Message invite = {
        .type = INVITATION,
    };

    strncpy(invite.roomname, message->roomname, sizeof(invite.roomname) - 1);
    invite.roomname[sizeof(invite.roomname) - 1] = '\0';

    strncpy(invite.username, connection->user->username, sizeof(invite.username) - 1);
    invite.username[sizeof(invite.username) - 1] = '\0';
    sendTo(toJSON(&invite), invited_connection);
}


    g_mutex_unlock(&(server->room_mutex));
}

void joinRoom(struct Server* server, Message* message, struct Connection* connection) {
    g_mutex_lock(&(server->room_mutex));
    
    // Verificar si la sala existe
    GHashTable* room_members = g_hash_table_lookup(server->chat_rooms, message->roomname);
    if (room_members == NULL) {
        Message response = {
            .type = RESPONSE,
            .operation = OP_JOIN_ROOM,
            .result = RE_NO_SUCH_ROOM,
        };
        strncpy(response.extra, message->roomname, sizeof(response.extra) - 1);
        response.extra[sizeof(response.extra) - 1] = '\0';
        sendTo(toJSON(&response), connection);
        g_mutex_unlock(&(server->room_mutex));
        return;
    }

    // Verificar si el usuario ha sido invitado
    const char* user_status = g_hash_table_lookup(room_members, connection->user->username);
    if (user_status == NULL || strcmp(user_status, "INVITED") != 0) {
        Message response = {
            .type = RESPONSE,
            .operation = OP_JOIN_ROOM,
            .result = RE_NOT_INVITED,
        };
        strncpy(response.extra, message->roomname, sizeof(response.extra) - 1);
        response.extra[sizeof(response.extra) - 1] = '\0';
        sendTo(toJSON(&response), connection);
        g_mutex_unlock(&(server->room_mutex));
        return;
    }

    g_hash_table_insert(room_members, g_strdup(connection->user->username), g_strdup("ACTIVE"));

    // Enviar notificación a todos los miembros de la sala
    Message joinNotification = {
        .type = JOINED_ROOM,
    };

    strncpy(joinNotification.roomname, message->roomname, sizeof(joinNotification.roomname) - 1);
    joinNotification.roomname[sizeof(joinNotification.roomname) - 1] = '\0';

    strncpy(joinNotification.username, connection->user->username, sizeof(joinNotification.username) - 1);
    joinNotification.username[sizeof(joinNotification.username) - 1] = '\0';
    sendRoomMessageToAll(room_members, &joinNotification, server, connection);

    Message response = {
        .type = RESPONSE,
        .operation = OP_JOIN_ROOM,
        .result = RE_SUCCESS,
    };
    strncpy(response.extra, message->roomname, sizeof(response.extra) - 1);
    response.extra[sizeof(response.extra) - 1] = '\0';
    sendTo(toJSON(&response), connection);

    g_mutex_unlock(&(server->room_mutex));
}

void sendRoomMessageToAll(GHashTable* room_members, Message* message, struct Server* server, struct Connection* connection) {
    GHashTableIter iter;
    gpointer key, value;
    g_hash_table_iter_init(&iter, room_members);

    while (g_hash_table_iter_next(&iter, &key, &value)) {
        const char* username = (const char*)key;
        struct Connection* conn = (struct Connection*)g_hash_table_lookup(server->connections, username);
        if (conn->acceptedSocketFD == connection->acceptedSocketFD) continue;
        if (conn) {
            sendTo(toJSON(message), conn);
        }
    }
}

void listRoomUsers(struct Server* server, Message* message, struct Connection* connection) {
    // Bloquea el mutex para asegurar que el acceso a la estructura de salas sea seguro.
    g_mutex_lock(&(server->room_mutex));

   // Verificar si la sala existe
    GHashTable* room_members = g_hash_table_lookup(server->chat_rooms, message->roomname);
    if (room_members == NULL) {
        Message response = {
            .type = RESPONSE,
            .operation = OP_ROOM_USERS,
            .result = RE_NO_SUCH_ROOM,
        };
        strncpy(response.extra, message->roomname, sizeof(response.extra) - 1);
        response.extra[sizeof(response.extra) - 1] = '\0';
        sendTo(toJSON(&response), connection);
        g_mutex_unlock(&(server->room_mutex));
        return;
    }

    // Verificar si el usuario ha sido invitado
    // const char* user_status = g_hash_table_lookup(room_members, connection->user->username);
    // if (user_status == NULL || strcmp(user_status, "INVITED") != 0) {
    //     Message response = {
    //         .type = RESPONSE,
    //         .operation = OP_ROOM_USERS,
    //         .result = RE_NOT_INVITED,
    //     };
    //     strncpy(response.extra, message->roomname, sizeof(response.extra) - 1);
    //     response.extra[sizeof(response.extra) - 1] = '\0';
    //     sendTo(toJSON(&response), connection);
    //     g_mutex_unlock(&(server->room_mutex));
    //     return;
    // }

    // Verificar si el usuario está en la sala
    if (!g_hash_table_contains(room_members, connection->user->username)) {
        Message response = {
            .type = RESPONSE,
            .operation = OP_ROOM_TEXT,
            .result = RE_NOT_JOINED,
        };
        strncpy(response.extra, message->roomname, sizeof(response.extra) - 1);
        response.extra[sizeof(response.extra) - 1] = '\0';
        sendTo(toJSON(&response), connection);
        g_mutex_unlock(&(server->room_mutex));
        return;
    }

    Message response = {
        .type = ROOM_USER_LIST,
        .chat_rooms = room_members,
    };

    strncpy(response.roomname, message->roomname, sizeof(response.roomname) - 1);
    response.roomname[sizeof(response.roomname) - 1] = '\0';

    printf("%s\n",toJSON(&response));

    sendTo(toJSON(&response), connection);

    g_mutex_unlock(&(server->room_mutex));
}

void sendRoomMessage(struct Server* server, Message* message, struct Connection* connection) {
    g_mutex_lock(&(server->room_mutex));
    
    // Verificar si la sala existe
    GHashTable* room_members = g_hash_table_lookup(server->chat_rooms, message->roomname);
    if (room_members == NULL) {
        Message response = {
            .type = RESPONSE,
            .operation = OP_ROOM_TEXT,
            .result = RE_NO_SUCH_ROOM,
        };
        strncpy(response.extra, message->roomname, sizeof(response.extra) - 1);
        response.extra[sizeof(response.extra) - 1] = '\0';
        sendTo(toJSON(&response), connection);
        g_mutex_unlock(&(server->room_mutex));
        return;
    }

    // Verificar si el usuario está en la sala
    if (!g_hash_table_contains(room_members, connection->user->username)) {
        Message response = {
            .type = RESPONSE,
            .operation = OP_ROOM_TEXT,
            .result = RE_NOT_JOINED,
        };
        strncpy(response.extra, message->roomname, sizeof(response.extra) - 1);
        response.extra[sizeof(response.extra) - 1] = '\0';
        sendTo(toJSON(&response), connection);
        g_mutex_unlock(&(server->room_mutex));
        return;
    }

    // Enviar el mensaje a todos los usuarios en la sala
    Message roomMessage = {
        .type = ROOM_TEXT_FROM,
    };

    strncpy(roomMessage.roomname, message->roomname, sizeof(roomMessage.roomname) - 1);
    roomMessage.roomname[sizeof(roomMessage.roomname) - 1] = '\0';
    
    strncpy(roomMessage.username, connection->user->username, sizeof(roomMessage.username) - 1);
    roomMessage.username[sizeof(roomMessage.username) - 1] = '\0';
    strncpy(roomMessage.text, message->text, sizeof(roomMessage.text) - 1);
    roomMessage.text[sizeof(roomMessage.text) - 1] = '\0';

    sendRoomMessageToAll(room_members, &roomMessage, server, connection);

    g_mutex_unlock(&(server->room_mutex));
}

void leaveRoom(struct Server* server, Message* message, struct Connection* connection) {
    g_mutex_lock(&(server->room_mutex));
    
    // Verificar si la sala existe
    GHashTable* room_members = g_hash_table_lookup(server->chat_rooms, message->roomname);
    if (room_members == NULL) {
        Message response = {
            .type = RESPONSE,
            .operation = OP_LEAVE_ROOM,
            .result = RE_NO_SUCH_ROOM,
        };
        strncpy(response.extra, message->roomname, sizeof(response.extra) - 1);
        response.extra[sizeof(response.extra) - 1] = '\0';
        sendTo(toJSON(&response), connection);
        g_mutex_unlock(&(server->room_mutex));
        return;
    }

    // Verificar si el usuario está en la sala
    if (!g_hash_table_contains(room_members, connection->user->username)) {
        Message response = {
            .type = RESPONSE,
            .operation = OP_LEAVE_ROOM,
            .result = RE_NOT_JOINED,
        };
        strncpy(response.extra, message->roomname, sizeof(response.extra) - 1);
        response.extra[sizeof(response.extra) - 1] = '\0';
        sendTo(toJSON(&response), connection);
        g_mutex_unlock(&(server->room_mutex));
        return;
    }

    // Eliminar al usuario de la sala
    g_hash_table_remove(room_members, connection->user->username);

    // Enviar notificación a la sala
    Message leaveNotification = {
        .type = LEFT_ROOM,
    };

    strncpy(leaveNotification.roomname, message->roomname, sizeof(leaveNotification.roomname) - 1);
    leaveNotification.roomname[sizeof(leaveNotification.roomname) - 1] = '\0';

    strncpy(leaveNotification.username, connection->user->username, sizeof(leaveNotification.username) - 1);
    leaveNotification.username[sizeof(leaveNotification.username) - 1] = '\0';
    sendRoomMessageToAll(room_members, &leaveNotification, server, connection);

    g_mutex_unlock(&(server->room_mutex));
}

void disconnectUser(struct Server* server, Message* message, struct Connection* connection) {
    Message serverResponse = {
        .type = DISCONNECTED,
    };
    strncpy(serverResponse.username, connection->user->username, sizeof(serverResponse.username) - 1);
    serverResponse.username[sizeof(serverResponse.username) - 1] = '\0';

    sendToGlobalChat(toJSON(&serverResponse), connection->acceptedSocketFD, server);
    remove_connection(server, connection);

}

