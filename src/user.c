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
    user->status = ONLINE;

    return user;
}
