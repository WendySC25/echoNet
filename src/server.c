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
        (GDestroyNotify)free  // Función para liberar los valores (estructuras Connection)
    );

    server->chat_rooms = g_hash_table_new_full(g_str_hash, g_str_equal, g_free, (GDestroyNotify) g_hash_table_destroy);
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
                free(clientSocket);
            }
        } else if (clientSocket) {
            free(clientSocket);
        }
    }
}

void freeConnection(struct Connection* connection) {
    fclose(connection->in);
    fclose(connection->out);
    close(connection->acceptedSocketFD);
    freeUser(connection->user);
    free(connection);
}


// void add_connection(Server *server, const char *username, Connection *conn) {
    
//     char *username_key = g_strdup(username);
//     g_hash_table_insert(server->connections, username_key, conn);
//     server->acceptedConnectionCount++;
// }

// Connection *get_connection(Server *server, const char *username) {
//     return g_hash_table_lookup(server->connections, username);
// }

// void remove_connection(Server *server, const char *username) {
//     // Obtener la conexión y eliminarla del diccionario
//     Connection *conn = g_hash_table_lookup(server->connections, username);
//     if (conn != NULL) {
//         g_hash_table_remove(server->connections, username);
//         free(conn); // Liberar la memoria de la conexión
//         server->acceptedConnectionCount--;
//     }
// }

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
            printf("Received message: %s", buffer);

            // //ALGO ESTA MAL EN .getMessage
            // size_t len = strlen(buffer);
            // if (len > 0 && buffer[len - 1] == '\n') {
            //     buffer[len - 1] = '\0';
            // }

            Message message = getMessage(buffer);
            switch (message.type) {

            case IDENTIFY:
                identifyUser(server, &message, connection);
                break;

            case INVALID:
                printf("%s", "WTF IS GOING HWRRE");
                break;

            default:
                printf("%s", "AHHHHHHHHH HELP ME");
                break;
            }

    
            // sendToGlobalChat(buffer, connection->acceptedSocketFD, server);
        } else {
            break; 
        }
    }

    fclose(connection->in);
    fclose(connection->out);
    close(connection->acceptedSocketFD);
    free(data);
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
