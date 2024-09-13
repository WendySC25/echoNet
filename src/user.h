#ifndef USER_H
#define USER_H

#define MAX_USERNAME_LENGTH 8
#include <glib.h>

typedef enum {
    ONLINE,
    AWAY,
    BUSY,
    UNKNOWN
} UserStatus;

const char* UserStatusToString(UserStatus status);
UserStatus getUserStatus(const char* str);

typedef struct {
    char            username[MAX_USERNAME_LENGTH + 1];  
    UserStatus      status;
} User;

User* newUser(const char *username);
void freeUser(User* user);

#endif
