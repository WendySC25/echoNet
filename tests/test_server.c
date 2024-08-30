#include "unity.h"
#include "server.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>

#define MAX_USERNAME_LENGTH 8

//Clases temporales para hacer pruebas unitarias

typedef enum {
    ONLINE,
    AWAY,
    BUSY
} UserStatus;

typedef struct {
    char username[MAX_USERNAME_LENGTH + 1];  
    UserStatus status; 
    int socketFD;      
} User;



void setUp(void) {
    // Se llama antes de cada prueba
}

void tearDown(void) {
    // Se llama después de cada prueba
}


int main(void) {

    UNITY_BEGIN();

    int socketFD = socket(AF_INET, SOCK_STREAM, 0);

    User userA;
    strncpy(userA.username, "Alice", sizeof(userA.username) - 1);
    userA.username[sizeof(userA.username) - 1] = '\0'; 
    userA.status = ONLINE;
    userA.socketFD = socketFD;

    struct sockaddr_in *address = malloc(sizeof(struct sockaddr_in));
    int port  = 2000;
    char *ip = "127.0.0.1";
    address->sin_family = AF_INET;
    address->sin_port = htons(port); 

    if (inet_pton(AF_INET, ip, &address->sin_addr.s_addr) != 1) {
            perror("inet_pton failed");
            free(address); 
            exit(EXIT_FAILURE);
    }   

    int result = connect(socketFD, (struct sockaddr *)address, sizeof(*address));
    free(address);

    if (result == 0)
        printf("Connection was successful\n");
    else {
        printf("Connection fail\n");
        exit(EXIT_FAILURE);
    }

    close(socketFD);




    //RUN_TEST();
    return UNITY_END();
}



