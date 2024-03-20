#ifndef SSL_CHAT_APP_NET_H
#define SSL_CHAT_APP_NET_H
#include "app.hpp"
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>

void *get_in_addr(struct sockaddr *);

class Socket
{
    ChatWindow *chat_window;
    bool running = false;

public:
    string user;
    int skt;
    sockaddr_in addr;
    Socket(ChatWindow *chat_window, int skt = -1);
    ~Socket();
    string receive_msg();
    string receive_ctl_msg();
    void send_msg(string msg);
    void send_ctl_msg(string msg);
    void run();
};

class Server
{
    int skt;
    uint32_t ip;
    int port;
    struct sockaddr server;
    struct addrinfo *servinfo;
    ChatWindow *chat_window;

public:
    Socket *socketObj;
    Server(uint32_t ip, int port, ChatWindow *chat_window);
    void start();
    bool handshake();
    void handle_client();
    void stop();
};

class Client
{
    int skt;
    uint32_t ip;
    int port;
    struct sockaddr server;
    ChatWindow *chat_window;
    struct addrinfo *servinfo;

public:
    Socket *socketObj;
    Client(uint32_t ip, int port, ChatWindow *chat_window);
    void start(string);
    bool handshake();
    void stop();
};
#endif
