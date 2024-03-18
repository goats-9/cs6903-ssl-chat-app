#ifndef SSL_CHAT_APP_APP_H
#define SSL_CHAT_APP_APP_H

#include "tui.hpp"
#include <thread>
enum UserRole
{
    SERVER,
    CLIENT,
};

class ChatApp
{
    ChatWindow *chatWindow = NULL;
    string user;
    string other_user;
    UserRole role;
    string hostname;
    uint8_t port;
    thread *network_thread;
    uint32_t ip;
    int Port;
    ChatWindow *chat_window;
    bool connected = false;

    vector<Message> messages = {
        {0, "user1", "hello"},
        {1, "user2", "hi"},
        {2, "user1", "how are you?"},
        {3, "user2", "I'm fine, thank you"},
        {4, "user1", "good to hear that"},
        {5, "user2", "how about you?"},
        {6, "user1", "I'm good"},
        {7, "user2", "that's great"},
        {8, "user1", "bye"},
        {9, "user2", "bye"}};

public:
    ChatApp(UserRole role, string user, string hostname, uint16_t port);
    ~ChatApp();
    void init_color();
    void create_window();
    void run();
    void run_client();
    void run_server();
    void handle_client();
    void run_tui();
    void stop_network_thread();
    void stop_client();
    void stop_server();
};

#endif // SSL_CHAT_APP_APP_H
