#ifndef CLIENT_H
#define CLIENT_H

#include <stdio.h>
#include <pthread.h>

/**
 * @brief Sends a message to the server.
 * @param connection Pointer to the connection structure.
 */
void sentMessageToServer(struct Connection* connection);

/**
 * @brief Creates a thread for receiving messages from the server.
 * @param connection Pointer to the connection structure.
 */
void createReceiveMessageThreadClient(struct Connection* connection);

/**
 * @brief Thread function for receiving messages from the server.
 * @param arg Pointer to the connection structure (passed as argument).
 * @return Pointer to the thread's return value.
 */
void *receiveMessageFromServer(void *arg);

/**
 * @brief Establishes a new connection to a server.
 * @param ip The IP address of the server.
 * @param port The port number to connect to.
 * @return Pointer to the new connection structure.
 */
struct Connection* connectNewClient(char *ip, int port);

/**
 * @brief Disconnects from the server and frees associated resources.
 * 
 * This function sends a disconnect message to the server, closes any open sockets, 
 * and frees the memory associated with the `Connection` structure. It also exits 
 * the program gracefully.
 * 
 * @param connection Pointer to the `Connection` structure to be disconnected.
 */
void disconnect(struct Connection* connection); 

/**
 * @brief Handles incoming messages from the server.
 * @param buffer Buffer containing the received message.
 * @param connection Pointer to the connection structure.
 */
void handleReceiveMessage(char* buffer, struct Connection* connection);

/**
 * @brief Handles the SIGINT signal (e.g., Ctrl+C).
 * @param sig The signal number.
 */
void handle_sigint(int sig);

#endif // CLIENT_H
