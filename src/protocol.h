#ifndef PROTOCOL_H
#define PROTOCOL_H

#include "message.h"
#include "server.h"

/**
 * @brief Identifies a user based on the provided message.
 * @param server Pointer to the server instance.
 * @param message Pointer to the message containing user identification info.
 * @param connection Pointer to the connection of the user being identified.
 */
void identifyUser(struct Server* server, Message* message, struct Connection* connection);

/**
 * @brief Changes the status of a user.
 * @param server Pointer to the server instance.
 * @param message Pointer to the message containing new status information.
 * @param connection Pointer to the connection of the user changing status.
 */
void changeStatus(struct Server* server, Message* message, struct Connection* connection);

/**
 * @brief Lists all users on the server.
 * @param server Pointer to the server instance.
 * @param connection Pointer to the connection requesting the user list.
 */
void listUsers(struct Server* server, struct Connection* connection);

/**
 * @brief Sends a private message to a specific user.
 * @param server Pointer to the server instance.
 * @param message Pointer to the message to be sent.
 * @param connection Pointer to the connection sending the message.
 */
void sendPrivateMessage(struct Server* server, Message* message, struct Connection* connection);

/**
 * @brief Sends a public message to all users.
 * @param server Pointer to the server instance.
 * @param message Pointer to the message to be sent.
 * @param connection Pointer to the connection sending the message.
 */
void sendPublicMessage(struct Server* server, Message* message, struct Connection* connection);

/**
 * @brief Creates a new chat room.
 * @param server Pointer to the server instance.
 * @param message Pointer to the message containing room creation details.
 * @param connection Pointer to the connection requesting room creation.
 */
void createRoom(struct Server* server, Message* message, struct Connection* connection);

/**
 * @brief Invites users to a chat room.
 * @param server Pointer to the server instance.
 * @param message Pointer to the message containing invitation details.
 * @param connection Pointer to the connection sending the invitation.
 */
void inviteToRoom(struct Server* server, Message* message, struct Connection* connection);

/**
 * @brief Allows a user to join a chat room.
 * @param server Pointer to the server instance.
 * @param message Pointer to the message containing room join request.
 * @param connection Pointer to the connection of the user joining the room.
 */
void joinRoom(struct Server* server, Message* message, struct Connection* connection);

/**
 * @brief Lists users in a specific chat room.
 * @param server Pointer to the server instance.
 * @param message Pointer to the message requesting the user list for a room.
 * @param connection Pointer to the connection requesting the list.
 */
void listRoomUsers(struct Server* server, Message* message, struct Connection* connection);

/**
 * @brief Sends a message to all members of a chat room.
 * @param server Pointer to the server instance.
 * @param message Pointer to the message to be sent.
 * @param connection Pointer to the connection sending the message.
 */
void sendRoomMessage(struct Server* server, Message* message, struct Connection* connection);

/**
 * @brief Allows a user to leave a chat room.
 * @param server Pointer to the server instance.
 * @param message Pointer to the message requesting to leave the room.
 * @param connection Pointer to the connection of the user leaving the room.
 */
void leaveRoom(struct Server* server, Message* message, struct Connection* connection);

/**
 * @brief Disconnects a user from the server.
 * @param server Pointer to the server instance.
 * @param connection Pointer to the connection of the user being disconnected.
 */
void disconnectUser(struct Server* server, struct Connection* connection);

/**
 * @brief Handles actions for an unidentified user.
 * @param connection Pointer to the connection of the unidentified user.
 */
void handlesUnidentifiedUser(struct Connection* connection);

#endif // PROTOCOL_H
