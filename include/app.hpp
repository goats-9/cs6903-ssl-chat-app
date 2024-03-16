#ifndef SSL_CHAT_APP_APP_H
#define SSL_CHAT_APP_APP_H

#include "tui.hpp"


class ChatApp {
    ChatWindow *chatWindow = NULL;
    string user;
    vector <Message> messages = {
        {0, "user1", "hello"},
        {1, "user2", "hi"},
        {2, "user1", "how are you?"},
        {3, "user2", "I'm fine, thank you"},
        {4, "user1", "good to hear that"},
        {5, "user2", "how about you?"},
        {6, "user1", "I'm good"},
        {7, "user2", "that's great"},
        {8, "user1", "bye"},
        {9, "user2", "bye"}
    };

    public:
    ChatApp();
    ~ChatApp();
    void init_color();
    void create_window();
    void run();
};

#endif //SSL_CHAT_APP_APP_H
