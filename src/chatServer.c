#include "server.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <arpa/inet.h>

int main(int argc, char* argv[]){

    if (argc < 2) {
        fprintf(stderr, "Usage: %s <Port>\n", argv[0]);
        return EXIT_FAILURE;
    }

    struct Server* server = newServer(atoi(argv[1]));
    if (server == NULL) {
        fprintf(stderr, "Failed to start server.\n");
        return EXIT_FAILURE;
    }
    printf("Server is up and running successfully on port %d :) \n", atoi(argv[1]));
    startServer(server);
    
    freeServer(server);
    return 0;
}
