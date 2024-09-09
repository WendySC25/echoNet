#include "user.h"
#include <string.h>
#include <stdlib.h>

User* newUser(const char *username) {
    User* user = malloc(sizeof(User));
    if (user == NULL) {
        return NULL;
    }
    strncpy(user->username, username, MAX_USERNAME_LENGTH - 1);
    user->username[MAX_USERNAME_LENGTH - 1] = '\0'; 

    strncpy(user->status, "online", MAX_USERNAME_LENGTH - 1);
    user->status[MAX_USERNAME_LENGTH - 1] = '\0'; 


    return user;
}

void freeUser(User* user) {
    free(user);
}
