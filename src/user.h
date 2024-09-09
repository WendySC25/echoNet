#ifndef USER_H
#define USER_H

#define MAX_USERNAME_LENGTH 8

typedef enum {
    ONLINE,
    AWAY,
    BUSY
} UserStatus;

typedef struct {
    char username[MAX_USERNAME_LENGTH + 1];  
    UserStatus status;     
} User;

User* newUser(const char *username);

#endif