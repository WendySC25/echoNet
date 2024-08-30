#ifndef SERVER_H
#define SERVER_H

#include <stdbool.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

struct AcceptedSocket {
    int    acceptedSocketFD;
    struct sockaddr_in address;
    int    error;
    bool   acceptedSuccessfully;
};

struct sockaddr_in* createAddress(int port);
struct AcceptedSocket* accepConnection(int serverSocketFD);
void start(int serverSocketFD);

#endif 
