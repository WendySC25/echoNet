#include "unity.h"
#include "server.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <pthread.h>

#define MAX_USERNAME_LENGTH 8

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

    int serverSocketFD = socket(AF_INET, SOCK_STREAM, 0); 
    struct sockaddr_in *serverAddress = createAddress(2000); 
    bind(serverSocketFD, (struct sockaddr *)serverAddress, sizeof(*serverAddress));
    listen(serverSocketFD, 10);
    
    pthread_t serverThread;

    if (pthread_create(&serverThread, NULL, (void *(*)(void *))start, (void *)(intptr_t)serverSocketFD) != 0) {
        perror("Failed to create server thread");
        exit(EXIT_FAILURE);
    }

    pthread_detach(serverThread);
    sleep(1); 
    
    RUN_TEST(test_socket_connection);

    shutdown(serverSocketFD, SHUT_RDWR);

    return UNITY_END();
}



