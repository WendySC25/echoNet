#include "unity.h"
#include "server.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <pthread.h>

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

User createUser(const char *username, UserStatus status, int socketFD) {
    User user;
    strncpy(user.username, username, MAX_USERNAME_LENGTH);
    user.username[MAX_USERNAME_LENGTH] = '\0';
    user.status = status;
    user.socketFD = socketFD;
    return user;
}

void setUp(void) {
    // stuff

}

void tearDown(void) {
    // stuff
}

void test_socket_connection(void) {

    int clientSocketFD = socket(AF_INET, SOCK_STREAM, 0);
    TEST_ASSERT_NOT_EQUAL(-1, clientSocketFD); 

    struct sockaddr_in *clientAddress = malloc(sizeof(struct sockaddr_in));
    TEST_ASSERT_NOT_NULL(clientAddress); 

    int port = 2000;
    char *ip = "127.0.0.1";
    clientAddress->sin_family = AF_INET;
    clientAddress->sin_port = htons(port);

    int inet_pton_result = inet_pton(AF_INET, ip, &clientAddress->sin_addr.s_addr);
    TEST_ASSERT_EQUAL(1, inet_pton_result); 

    int connect_result = connect(clientSocketFD, (struct sockaddr *)clientAddress, sizeof(*clientAddress));
    TEST_ASSERT_EQUAL(0, connect_result); 

    free(clientAddress); 
    close(clientSocketFD); 
    
}


int main(void) {

    UNITY_BEGIN();

    RUN_TEST(test_socket_connection);

    return UNITY_END();
}



