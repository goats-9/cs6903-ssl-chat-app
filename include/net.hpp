#ifndef SSL_CHAT_APP_NET_H
#define SSL_CHAT_APP_NET_H
#include "app.hpp"
#include <bits/stdc++.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>


class Socket
{
    ChatWindow *chat_window;
    bool running=false;

public:
    string user;
    int skt;
    Socket(ChatWindow *chat_window, int skt = -1);
    ~Socket();
    string receive_msg();
    void send_msg(string msg);
    void run();
};

class Server
{
    int skt;
    uint32_t ip;
    int port;
    struct sockaddr_in server;
    ChatWindow *chat_window;

public:
    Server(uint32_t ip, int port, ChatWindow *chat_window);
    void start();
    bool handshake(int client_skt);
    void handle_client(int client_skt);
    void stop();
};

class Client
{
    int skt;
    uint32_t ip;
    int port;
    struct sockaddr_in server;
    ChatWindow *chat_window;

public:
    Client(uint32_t ip, int port, ChatWindow *chat_window);
    void start();
    bool handshake();
};

int server_init();
int client_init();
void close_socket(int);
int send_msg(int, char *);
char *recv_msg(int);
int send_ctrl_msg(int, char *);
char *recv_ctrl_msg(int);
#endif
