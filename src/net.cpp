#include "net.hpp"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>

#define PORT_NUM 8080
#define PORT "8080"
#define BUFFER_SIZE 1024

// get sockaddr, IPv4 or IPv6:
void *get_in_addr(struct sockaddr *sa)
{
    if (sa->sa_family == AF_INET) {
        return &(((struct sockaddr_in*)sa)->sin_addr);
    }

    return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

Socket::Socket(ChatWindow *chat_window, int skt)
{
    this->chat_window = chat_window;
    if (skt == -1)
    {
        if (this->skt = socket(AF_INET, SOCK_DGRAM, 0)) {
            perror("socket: socket");
            exit(1);
        }
    }
    else
    {
        this->skt = skt;
    }
}

Socket::~Socket()
{
    close(skt);
}

string Socket::receive_msg()
{
    char buffer[BUFFER_SIZE];
    if (recv(skt, buffer, BUFFER_SIZE, 0) == -1) {
        perror("socket: receive_msg");
        exit(1);
    }
    return string(buffer);
}

string Socket::receive_ctl_msg() {
    string recv_msg = Socket::receive_msg();
    // Send ACK
    send_msg("ACK");
}

void Socket::send_msg(string msg)
{
    if (send(skt, msg.c_str(), msg.size(), 0) == -1) {
        perror("socket: send_msg");
        exit(1);
    }
}

void Socket::send_ctl_msg(string msg) {
    Socket::send_msg(msg);
    // Wait for ACK
    while (Socket::receive_msg() != "ACK");
    return;
}

void Socket::run()
{
    running = true;
    while (running)
    {
        string msg = receive_msg();
        chat_window->message_received(user, msg);
    }
}

Server::Server(uint32_t ip, int port, ChatWindow *chat_window)
{
    this->chat_window = chat_window;
    this->ip = ip;
    this->port = port;
    skt = socket(AF_INET, SOCK_DGRAM, 0);
}

void Server::start()
{
    struct addrinfo hints;
    int rv;
    int numbytes;
    char buf[BUFFER_SIZE];
    socklen_t addr_len;
    char s[INET6_ADDRSTRLEN];

    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_INET; // set to AF_INET to use IPv4
    hints.ai_socktype = SOCK_DGRAM;
    hints.ai_flags = AI_PASSIVE; // use my IP

    if ((rv = getaddrinfo(NULL, PORT, &hints, &this->servinfo)) != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
        exit(1);
    }
}

void Server::handle_client(int client_skt)
{
    struct addrinfo *p;
    // loop through all the results and bind to the first we can
    for(p = this->servinfo; p != NULL; p = p->ai_next) {
        if ((this->skt = socket(p->ai_family, p->ai_socktype,
                p->ai_protocol)) == -1) {
            perror("listener: socket");
            continue;
        }

        if (bind(this->skt, p->ai_addr, p->ai_addrlen) == -1) {
            close(this->skt);
            perror("listener: bind");
            continue;
        }

        break;
    }

    if (p == NULL) {
        fprintf(stderr, "listener: failed to bind socket\n");
        exit(2);
    }

    freeaddrinfo(this->servinfo);
    Server::handshake(this->skt);
    Socket client(chat_window, this->skt);
    client.user = "client";
    client.run();
}

bool Server::handshake(int client_skt)
{
    // TODO: implement handshake
    return true;
}

void Server::stop()
{
    close(this->skt);
}

Client::Client(uint32_t ip, int port, ChatWindow *chat_window)
{
    this->chat_window = chat_window;
    this->ip = ip;
    this->port = port;
    skt = socket(AF_INET, SOCK_STREAM, 0);
}

void Client::start(char *server_name)
{
    struct addrinfo hints, *p;
    int rv;

    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_INET; // set to AF_INET to use IPv4
    hints.ai_socktype = SOCK_DGRAM;

    if ((rv = getaddrinfo(server_name, PORT, &hints, &this->servinfo)) != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
        exit(1);
    }

    // loop through all the results and make a socket
    for(p = this->servinfo; p != NULL; p = p->ai_next) {
        if ((this->skt = socket(p->ai_family, p->ai_socktype,
                p->ai_protocol)) == -1) {
            perror("talker: socket");
            continue;
        }

        break;
    }

    if (p == NULL) {
        fprintf(stderr, "talker: failed to create socket\n");
        exit(2);
    }

    freeaddrinfo(servinfo);
    Client::handshake();
    Socket client(chat_window, this->skt);
    client.user = "Server";
    client.run();
}

bool Client::handshake()
{
    // TODO: implement handshake
    return true;
}
