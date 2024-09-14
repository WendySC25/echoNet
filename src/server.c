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
  if(strlen(ip) ==0) address->sin_addr.s_addr = INADDR_ANY;
  else inet_pton(AF_INET,ip,&address->sin_addr.s_addr);
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
					      g_str_hash,         
					      g_str_equal,          
					      g_free,               
					      freeConnection  
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

  connection->in  = fdopen(clientSocketFD, "r");
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
                
	createReceiveMessageThread(clientSocket, server);
      } else {
	printf("Maximum number of connections reached.\n");

      }
    } else if (clientSocket) {
      //
    }
  }
}

void freeConnection(gpointer data) {
  struct Connection *connection = (struct Connection *)data;
  if (connection != NULL) {
    fclose(connection->in);
    fclose(connection->out);
    close(connection->acceptedSocketFD);
    if(connection->user != NULL) freeUser(connection->user);
    //free(connection); //provoca Segmentatio fault
  }
}

void addConnectionn(struct Server *server, const char *username, struct Connection *conn) {
  g_mutex_lock(&(server->room_mutex));

  char *username_key = g_strdup(username);
  g_hash_table_insert(server->connections, username_key, conn);
  server->acceptedConnectionCount++;

  g_mutex_unlock(&(server->room_mutex));
}

void removeConnection(struct Server *server, struct Connection *conn) {
  g_mutex_lock(&(server->room_mutex));

  gboolean removed = g_hash_table_remove(server->connections, conn->user->username);
  if (removed) server->acceptedConnectionCount--;

  g_mutex_unlock(&(server->room_mutex));
}

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
	handlesUnidentifiedUser(connection);
      }

      switch (message.type) {

      case IDENTIFY:
	identifyUser(server, &message, connection);
                
	break;

      case STATUS:
	changeStatus(server, &message, connection);
	break;
            
      case USERS:
	listUsers(server, connection);
	break;
            
      case TEXT:
	sendPrivateMessage(server, &message, connection);
	break;

      case PUBLIC_TEXT:
	sendPublicMessage(server, &message, connection);
	break;
            
      case DISCONNECT:
	disconnectUser(server, connection);
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

      default:
	printf("%s\n", "MENSAJE NO RECONOCIDO");
  disconnectUser(server, connection);
	break;
      }

    } else {
      break; 
    }
  }

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

void sendRoomMessageToAll(GHashTable* room_members, char *buffer, struct Server* server, struct Connection* connection) {
  GHashTableIter iter;
  gpointer key, value;
  g_hash_table_iter_init(&iter, room_members);

  while (g_hash_table_iter_next(&iter, &key, &value)) {

    const char* username = (const char*)key;
    const char* status = (const char*)value;

    struct Connection* conn = (struct Connection*)g_hash_table_lookup(server->connections, username);
    if (conn->acceptedSocketFD == connection->acceptedSocketFD) continue;
    if (strcmp(status, "INVITED") != 0) 
      if (conn) sendTo(buffer, conn);
            
    
  }
}

void freeServer(struct Server* server) {
  if (server == NULL) {
    return;
  }

  if (server->connections != NULL) {
    g_hash_table_unref(server->connections);
  }

  if (server->chat_rooms != NULL) {
    g_hash_table_unref(server->chat_rooms);
  }

  g_mutex_clear(&(server->room_mutex));

  if (server->address != NULL) {
    free(server->address);
  }

  close(server->serverSocketFD);

  free(server);
}
