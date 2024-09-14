#ifndef USER_H
#define USER_H

#include <glib.h>

#define MAX_USERNAME_LENGTH 8

/**
 * @brief User status types.
 */
typedef enum {
    ONLINE,     /**< Online. */
    AWAY,       /**< Away. */
    BUSY,       /**< Busy. */
    UNKNOWN     /**< Unknown. */
} UserStatus;

/**
 * @brief Converts a user status to a string.
 * @param status The user status.
 * @return String representation of the status.
 */
const char* UserStatusToString(UserStatus status);

/**
 * @brief Converts a string to a user status.
 * @param str String representing the status.
 * @return Corresponding user status.
 */
UserStatus getUserStatus(const char* str);

/**
 * @brief Structure defining a user.
 */
typedef struct {
    char username[MAX_USERNAME_LENGTH + 1]; /**< Username. */
    UserStatus status;                      /**< User status. */
} User;

/**
 * @brief Creates a new user.
 * @param username Username of the new user.
 * @return Pointer to the new user, or NULL on error.
 */
User* newUser(const char *username);

/**
 * @brief Frees memory associated with a user.
 * @param user Pointer to the user.
 */
void freeUser(User* user);

#endif // USER_H
