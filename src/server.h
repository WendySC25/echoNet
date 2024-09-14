#ifndef SERVER_H
#define SERVER_H

#include <stdbool.h>
#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "user.h"
#include "message.h"
#include <glib.h>

#define MAX_CONNECTIONS 100


// Ideally, the connection structure and all related methods should be defined in a separate class. For now, it will remain as is.

/**
 * @brief Represents a connection to a client.
 */
struct Connection {
    int    acceptedSocketFD;      
    int    error;                 
    bool   acceptedSuccessfully;  
    FILE   *in;                   
    FILE   *out;                  
    User   *user;                 
};

/**
 * @brief Represents the server instance.
 */
struct Server {
    int                 serverSocketFD;  
    struct sockaddr_in  *address;        
    GHashTable          *connections;    
    GHashTable          *chat_rooms;     
    GMutex              room_mutex;      
    int                 acceptedConnectionCount; 
};

/**
 * @brief Data passed to thread functions.
 */
struct ThreadData {
    struct Connection   *connection;
    struct Server       *server;     
};

/**
 * @brief Creates a socket address structure.
 * @param ip IP address for the server.
 * @param port Port number for the server.
 * @return Pointer to the created sockaddr_in structure.
 */
struct sockaddr_in *createAddress(char *ip, int port);

/**
 * @brief Creates a new server instance.
 * @param port Port number for the server.
 * @return Pointer to the new server instance.
 */
struct Server *newServer(int port);

/**
 * @brief Frees memory associated with a server.
 * @param server Pointer to the server instance.
 */
void freeServer(struct Server* server);

/**
 * @brief Accepts a new connection to the server.
 * @param server Pointer to the server instance.
 * @return Pointer to the new connection instance.
 */
struct Connection *acceptConnection(struct Server* server);

/**
 * @brief Creates a new connection instance.
 * @param clientSocketFD File descriptor for the client socket.
 * @return Pointer to the new connection instance.
 */
struct Connection *newConnection(int clientSocketFD);

/**
 * @brief Frees memory associated with a connection.
 * @param data Pointer to the connection instance.
 */
void freeConnection(gpointer data);

/**
 * @brief Starts the server and listens for connections.
 * @param server Pointer to the server instance.
 */
void startServer(struct Server* server);

/**
 * @brief Creates and starts a thread for receiving messages.
 * @param pSocket Pointer to the connection instance.
 * @param server Pointer to the server instance.
 */
void createReceiveMessageThread(struct Connection *pSocket, struct Server* server);

/**
 * @brief Thread function for receiving messages.
 * @param arg Pointer to ThreadData structure.
 * @return NULL.
 */
void *receiveMessages(void *arg);

/**
 * @brief Sends a message to all members in the global chat.
 * @param buffer Message to be sent.
 * @param socketFD File descriptor for the sender socket.
 * @param server Pointer to the server instance.
 */
void sendToGlobalChat(char *buffer, int socketFD, struct Server* server);

/**
 * @brief Sends a message to a specific connection.
 * @param buffer Message to be sent.
 * @param connection Pointer to the connection instance.
 */
void sendTo(char *buffer, struct Connection* connection);

/**
 * @brief Sends a message to all members of a specific chat room.
 * @param room_members Hash table of chat room members.
 * @param buffer Message to be sent.
 * @param server Pointer to the server instance.
 * @param connection Pointer to the connection instance.
 */
void sendRoomMessageToAll(GHashTable* room_members, char *buffer, struct Server* server, struct Connection* connection);

/**
 * @brief Adds a new connection to the server.
 * @param server Pointer to the server instance.
 * @param username Username of the new connection.
 * @param conn Pointer to the connection instance.
 */
void addConnectionn(struct Server *server, const char *username, struct Connection *conn);

/**
 * @brief Removes a connection from the server.
 * @param server Pointer to the server instance.
 * @param connection Pointer to the connection instance.
 */
void removeConnection(struct Server *server, struct Connection* connection);

#endif // SERVER_H
