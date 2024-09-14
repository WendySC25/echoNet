#include "user.h"
#include <string.h>
#include <stdlib.h>

const char* UserStatusToString(UserStatus status) {
    switch (status) {
        case ONLINE: return "ONLINE";
        case AWAY:   return "AWAY";
        case BUSY:   return "BUSY";
        default:     return "UNKNOWN"; 
    }
}

UserStatus getUserStatus(const char* str) {
    if (strcmp(str, "ONLINE") == 0)    return ONLINE;
    else if (strcmp(str, "AWAY") == 0) return AWAY;
    else if (strcmp(str, "BUSY") == 0) return BUSY;
    else return UNKNOWN; 
    
}

User* newUser(const char *username) {
    User* user = malloc(sizeof(User));
    if (user == NULL) 
        return NULL;
    strncpy(user->username, username, MAX_USERNAME_LENGTH - 1);
    user->username[MAX_USERNAME_LENGTH - 1] = '\0'; 
    user->status =  ONLINE;
    return user;
}

void freeUser(User* user) {
    free(user);
}
