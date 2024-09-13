#include "server.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include "user.h"
#include "protocol.h"
#include "message.h"

struct sockaddr_in* createAddress(char *ip, int port) {
    struct sockaddr_in  *address = malloc(sizeof(struct sockaddr_in));
    address->sin_family = AF_INET;
    address->sin_port = htons(port);

    if(strlen(ip) ==0)
        address->sin_addr.s_addr = INADDR_ANY;
    else
        inet_pton(AF_INET,ip,&address->sin_addr.s_addr);

    return address;
}

struct Server* newServer(int port) {
    struct Server* server = malloc(sizeof(struct Server));

    if (server == NULL) {
        perror("Failed to allocate memory for Server");
        return NULL;
    }

    server->serverSocketFD = socket(AF_INET, SOCK_STREAM, 0); 
    if (server->serverSocketFD < 0) {
        perror("Failed to create socket");
        free(server);
        return NULL;
    }

    server->address = createAddress("",port);
    if (server->address == NULL) {
        close(server->serverSocketFD);
        free(server);
        return NULL;
    }

    int result = bind(server->serverSocketFD, (struct sockaddr *)server->address, sizeof(*server->address));
    if (result != 0) {
        perror("Failed to bind socket");
        close(server->serverSocketFD);
        free(server->address);
        free(server);
        return NULL;
    }

    if (listen(server->serverSocketFD, 10) != 0) {
        perror("Failed to listen on socket");
        close(server->serverSocketFD);
        free(server->address);
        free(server);
        return NULL;
    }

    server->acceptedConnectionCount = 0;
    server->connections = g_hash_table_new_full(
        g_str_hash,           // Función de hash para cadenas
        g_str_equal,          // Función de comparación para cadenas
        g_free,               // Función para liberar las claves (serán cadenas duplicadas)
        freeConnection  // Función para liberar los valores (estructuras Connection)
    );

    server->chat_rooms = g_hash_table_new_full(
        g_str_hash, 
        g_str_equal, 
        g_free, 
        (GDestroyNotify) g_hash_table_destroy
    );

    g_mutex_init(&(server->room_mutex));
    
    return server;
}

struct Connection* acceptConnection(struct Server* server) {
    struct sockaddr_in clientAddress;
    socklen_t clientAddressSize = sizeof(clientAddress);
    int clientSocketFD = accept(server->serverSocketFD, (struct sockaddr *)&clientAddress, &clientAddressSize);

    if (clientSocketFD < 0) {
        perror("Failed to accept connection");
        return NULL;
    }

    struct Connection* connection = newConnection(clientSocketFD);
    if (connection == NULL) {
        return NULL;
    }

    return connection;
}

struct Connection* newConnection(int clientSocketFD) {
    struct Connection* connection = malloc(sizeof(struct Connection));
    if (connection == NULL) {
        perror("Failed to allocate memory for Connection");
        close(clientSocketFD);
        return NULL;
    }

    connection->acceptedSocketFD = clientSocketFD;
    connection->acceptedSuccessfully = clientSocketFD > 0;

    if (!connection->acceptedSuccessfully) {
        connection->error = clientSocketFD;
    }

    connection->in = fdopen(clientSocketFD, "r");
    connection->out = fdopen(clientSocketFD, "w");

    if (connection->in == NULL || connection->out == NULL) {
        perror("Failed to create FILE streams");
        close(clientSocketFD);
        free(connection);
        return NULL;
    }

    connection->user = newUser("newuser");
    return connection;
}

void startServer(struct Server* server) {
    while (1) {
        struct Connection* clientSocket = acceptConnection(server);
        if (clientSocket && clientSocket->acceptedSuccessfully) {
            if (server->acceptedConnectionCount < MAX_CONNECTIONS) {
                
                // server->connections[server->acceptedConnectionCount++] = *clientSocket;
                createReceiveMessageThread(clientSocket, server);
            } else {
                printf("Maximum number of connections reached.\n");
                // // free(clientSocket);
            }
        } else if (clientSocket) {
            // //free(clientSocket);
        }
    }
}

void freeConnection(gpointer data) {
    struct Connection *connection = (struct Connection *)data;
    if (connection != NULL) {
        // printf(" ESTO ESTA EN FREECONECTION %p", connection);
        fclose(connection->in);
        fclose(connection->out);
        close(connection->acceptedSocketFD);
        freeUser(connection->user);
        // printf(" ESTO ESTA EN FREECONECTION ANTES DEL ERROR %p", connection);
        //free(connection); //provoca Segmentatio fault
    }
}

void add_connection(struct Server *server, const char *username, struct Connection *conn) {
    g_mutex_lock(&(server->room_mutex));

    char *username_key = g_strdup(username);
    g_hash_table_insert(server->connections, username_key, conn);
    server->acceptedConnectionCount++;

    g_mutex_unlock(&(server->room_mutex));
}

// Connection *get_connection(Server *server, const char *username) {
//     return g_hash_table_lookup(server->connections, username);
// }

void remove_connection(struct Server *server, struct Connection *conn) {
    if (server == NULL) {
        printf("Error: El puntero server es nulo.\n");
        return;
    }

    if (conn == NULL) {
        printf("Error: El puntero conn es nulo.\n");
        return;
    }

    if (conn->user == NULL) {
        printf("Error: El puntero conn->user es nulo.\n");
        return;
    }

    if (conn->user->username == NULL) {
        printf("Error: El puntero conn->user->username es nulo.\n");
        return;
    }

    printf("LLEGAMOS AQUI\n");
    printf(" ESTO ESTA EN REMOVE %p", conn);

    g_mutex_lock(&(server->room_mutex));


    gboolean removed = g_hash_table_remove(server->connections, conn->user->username);

    if (removed) {
        printf("LLEGAMOS AQUI 2\n");
        server->acceptedConnectionCount--;
    } else {
        // printf("Error: La clave no se encontró en la tabla hash.\n");
    }

    g_mutex_unlock(&(server->room_mutex));
}
// void free_server(Server *server) {
//     g_hash_table_destroy(server->connections); 
//     free(server->address);
//     close(server->serverSocketFD);
// }


void createReceiveMessageThread(struct Connection *pSocket, struct Server* server) {
    pthread_t id;
    struct ThreadData *threadData = malloc(sizeof(struct ThreadData));
    if (threadData == NULL) {
        perror("Failed to allocate memory for threadData");
        return;
    }
    threadData->connection = pSocket;
    threadData->server     = server;

    if (pthread_create(&id, NULL, receiveMessages, threadData) != 0) {
        perror("Failed to create thread");
        free(threadData);
    }

    pthread_detach(id);
}

void *receiveMessages(void *arg) {
    struct ThreadData *data = (struct ThreadData *)arg;
    struct Connection *connection = data->connection;
    struct Server *server = data->server;

    char buffer[1024];

    while (1) {
        if (fgets(buffer, sizeof(buffer), connection->in) != NULL) {
            printf("Received message: %s\n", buffer);

            Message message = getMessage(buffer);

            if(message.type != IDENTIFY && (strcmp(connection->user->username, "newuser") == 0)){
                handlesUnidentifiedUser(server, &message, connection);
                continue;
            }

            switch (message.type) {

            case IDENTIFY:
                identifyUser(server, &message, connection);
                
                break;

            case STATUS:
                changeStatus(server, &message, connection);
                break;
            
            case USERS:
                listUsers(server, &message, connection);
                break;
            
            case TEXT:
                sendPrivateMessage(server, &message, connection);
                break;

            case PUBLIC_TEXT:
                sendPublicMessage(server, &message, connection);
                break;
            
            case DISCONNECT:
                disconnectUser(server, &message, connection);
                break;

            case NEW_ROOM:
                createRoom(server, &message, connection);
                break;

            case INVITE:
                inviteToRoom(server, &message, connection);
                break;
            
            case JOIN_ROOM:
                joinRoom(server, &message, connection);
                break;
            
            case ROOM_TEXT:
                sendRoomMessage(server, &message, connection);
                break;
            
            case ROOM_USERS:
                listRoomUsers(server, &message, connection);
                break;
            
            case LEAVE_ROOM:
                leaveRoom(server, &message, connection);
                break;

            // case INVALID:
            //     printf("%s", "WTF IS GOING HWRRE");
            //     break;

            default:
                printf("%s", "AHHHHHHHHH HELP ME");
                break;
            }

        } else {
            break; 
        }
    }

    // free(data);
    return NULL;
}

void sendToGlobalChat(char *buffer, int socketFD, struct Server *server) {
    GHashTableIter iter;
    gpointer key, value;
    g_hash_table_iter_init(&iter, server->connections);

    while (g_hash_table_iter_next(&iter, &key, &value)) {
        struct Connection *conn = (struct Connection *)value;
        if (conn->acceptedSocketFD != socketFD) {
            fprintf(conn->out, "%s\n", buffer);
            fflush(conn->out);
        }
    }
}

void sendTo(char *buffer, struct Connection* connection) {
    fprintf(connection->out, "%s\n", buffer);
    fflush(connection->out);  
}
