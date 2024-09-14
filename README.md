# echoNet
This project involves developing a chat server implemented in the C programming language. The server is designed to handle multiple client connections simultaneously, utilizing threads to ensure smooth and efficient communication.

## Dependencies

- **glib**: A utility library for C programming that provides data structures, macros, and other utilities.
- **cJSON**: A lightweight JSON parser in C.

## Building the Project

**`sudo apt-get install meson ninja-build`**
**`meson setup builddir`**
**`meson compile -C builddir`**
**`./builddir/servidorR <port>`**
**`./builddir/clienteR <ip> <port>`**

## Using Commands in the Chat

The chat system supports a variety of commands for interacting with the server. Below are the commands you can use and how they are parsed by the system.

### Supported Commands

1. **`\identify|<username>`**

    Registers a new username.
    - **Example:** `\identify|Juan`
    - **Description:** The argument is the username you wish to register. The username is unique.

2. **`\whisper|<username>|<message>`**

   Sends a private message to a specific user.
   - **Example:** `\whisper|Alice|Hello, how are you?`
   - **Description:** The first argument after `\whisper` is the username of the recipient, and the rest of the message is the content of the private message.

3. **`\broadcast|<message>`**

   Sends a message to all connected users.
   - **Example:** `\broadcast|Attention: Scheduled maintenance tonight.`
   - **Description:** The message will be visible to all users in the chat.

4. **`\getUsers`**

   Requests a list of all connected users.
   - **Example:** `\getUsers`
   - **Description:** This command does not require any arguments and returns a list of currently online users.

5. **`\setStatus|<status>`**

   Changes your user status. The status can be one of the following:
   - **AWAY**
   - **BUSY**
   - **ONLINE**
   - **Example:** `\setStatus|ONLINE`
   - **Description:** The `<status>` argument should be one of the predefined statuses. If an invalid status is provided, the system will not update your status.

6. **`\newRoom|<room name>`**

   Creates a new chat room.
   - **Example:** `\newRoom|GameRoom`
   - **Description:** The room name is specified in the argument following the command.

7. **`\invite|<room name>|<user1>|<user2>|...`**

   Invites one or more users to a specific room.
   - **Example:** `\invite|GameRoom|Bob|Carol`
   - **Description:** The first argument is the room name, and the subsequent arguments are the usernames to invite.

8. **`\acceptInvitation|<room name>`**

   Accepts an invitation to join a chat room.
   - **Example:** `\acceptInvitation|GameRoom`
   - **Description:** The argument is the name of the room you want to join.

9. **`\messageRoom|<room name>|<message>`**

   Sends a message to all members of a room.
   - **Example:** `\messageRoom|GameRoom|Welcome everyone!`
   - **Description:** The first argument is the room name, and the rest is the message that will be sent to all members of that room.

10. **`\getRoomUsers|<room name>`**

   Requests a list of users in a specific room.
   - **Example:** `\getRoomUsers|GameRoom`
   - **Description:** The argument is the name of the room for which you want to request the user list.

11. **`\leaveRoom|<room name>`**

    Leaves a chat room.
    - **Example:** `\leaveRoom|GameRoom`
    - **Description:** The argument is the name of the room you wish to leave.

12. **`\bye`**

    Disconnects the user from the chat.
    - **Example:** `\bye`
    - **Description:** This command does not require any arguments and disconnects the user from the server.



### Important Notes

- **Handling Spaces:** If the room name or message contains spaces, ensure that the arguments are properly delimited. Use the pipe character `|` to separate arguments.
- **Command Validation:** If a command is not recognized or the arguments are invalid, the system will return an error message.

