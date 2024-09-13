// #include <glib.h>
// #include "chatRoom.h"


// ChatRoom* createChatRoom(const gchar *roomname, const gchar *creator_username) {
//     ChatRoom *chatRoom = malloc(sizeof(ChatRoom));
//     if (chatRoom == NULL) {
//         perror("Failed to allocate memory for ChatRoom");
//         return NULL;
//     }

//     chatRoom->roomname = g_strdup(roomname);
//     chatRoom->members = g_hash_table_new_full(g_str_hash, g_str_equal, g_free, NULL);

//     // Agregar el creador de la sala como miembro inicial
//     g_hash_table_insert(chatRoom->members, g_strdup(creator_username), g_strdup("ACTIVE"));

//     return chatRoom;
// }


// void freeChatRoom(ChatRoom *chatRoom) {
//     if (chatRoom != NULL) {
//         if (chatRoom->members != NULL) {
//             g_hash_table_destroy(chatRoom->members);
//         }
//         if (chatRoom->roomname != NULL) {
//             g_free(chatRoom->roomname);
//         }

//         free(chatRoom);
//     }
// }
