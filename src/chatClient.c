
#include "client.h"
#include <signal.h>
#include <stdlib.h>

int main(int argc, char* argv[]) {

    if (argc != 3) {
        fprintf(stderr, "Usage: %s <IP> <Port>\n", argv[0]);
        return EXIT_FAILURE;
    }

    signal(SIGINT, handle_sigint);

    struct Connection* connection = connetNewClient(argv[1], atoi(argv[2]));
    createReceiveMessageThreadClient(connection);
    sentMessageToServer(connection);

    return 0;
}
