#ifndef USER_H
#define USER_H

#define MAX_USERNAME_LENGTH 8
#include <glib.h>

// typedef enum {
//     ONLINE,
//     AWAY,
//     BUSY
// } UserStatus;

typedef struct {
    char            username[MAX_USERNAME_LENGTH + 1];  
    char            status[MAX_USERNAME_LENGTH + 1]; 
    GHashTable      *joined_rooms;   

} User;

User* newUser(const char *username);
void freeUser(User* user);

#endif
