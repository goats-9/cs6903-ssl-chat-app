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

using namespace std;

#define PORT_NUM 8080
#define PORT "8080"
#define BUFFER_SIZE 1024

// int serverSocket;
// int clientSocket;

// void start_server()
// {
//     serverSocket = socket(AF_INET, SOCK_STREAM, 0);

//     struct sockaddr_in serverAddr;
//     serverAddr.sin_family = AF_INET;
//     serverAddr.sin_addr.s_addr = INADDR_ANY;
//     serverAddr.sin_port = htons(PORT_NUM);

//     bind(serverSocket, (struct sockaddr *)&serverAddr, sizeof(serverAddr));
//     listen(serverSocket, 5);

//     int newSocket = accept(serverSocket, nullptr, nullptr);
//     /* create a new thread for each connection*/
//     char msg[BUFFER_SIZE];
//     /*'msg' from received_message func*/
//     receive_msg(newSocket, msg);

// }
// string receive_msg(int socket){
//     recv(socket, buffer, BUFFER_SIZE,0);
//     string val = buffer;
//     return val;
// }

// void start_client()
// {
//     clientSocket = socket(AF_INET, SOCK_STREAM, 0);
//     struct sockaddr_in serverAddress;
//     serverAddress.sin_family = AF_INET;
//     serverAddress.sin_addr.s_addr = INADDR_ANY;
//     serverAddress.sin_port = htons(8080);

//     connect(clientSocket, (struct sockaddr *)&serverAddress, sizeof(serverAddress));
//     char *message = "hello I am client";
//     /* 'message' is the msg from send_message func*/
//     send(clientSocket, message, strlen(message), 0);
// }
// void server_stop()
// {
//     close(serverSocket);
// }
// void client_stop()
// {
//     close(clientSocket);
// }

Socket::Socket(ChatWindow *chat_window, int skt)
{
    this->chat_window = chat_window;
    if (skt == -1)
    {
        this->skt = socket(AF_INET, SOCK_STREAM, 0);
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
    recv(skt, buffer, BUFFER_SIZE, 0);
    return string(buffer);
}

void Socket::send_msg(string msg)
{
    send(skt, msg.c_str(), msg.size(), 0);
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
    skt = socket(AF_INET, SOCK_STREAM, 0);
}

void Server::start()
{
    server.sin_family = AF_INET;
    server.sin_addr.s_addr = ip;
    server.sin_port = htons(port);

    bind(skt, (struct sockaddr *)&server, sizeof(server));
    listen(skt, 5);

    int client_skt = accept(skt, nullptr, nullptr);
    handle_client(client_skt);
}

void Server::handle_client(int client_skt)
{
    handshake(client_skt);
    Socket client(chat_window, client_skt);
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
    close(skt);
}

Client::Client(uint32_t ip, int port, ChatWindow *chat_window)
{
    this->chat_window = chat_window;
    this->ip = ip;
    this->port = port;
    skt = socket(AF_INET, SOCK_STREAM, 0);
}

void Client::start()
{
    server.sin_family = AF_INET;
    server.sin_addr.s_addr = ip;
    server.sin_port = htons(port);

    connect(skt, (struct sockaddr *)&server, sizeof(server));
    handshake();
    Socket client(chat_window, skt);
    client.user = "Server";
    client.run();
}

bool Client::handshake()
{
    // TODO: implement handshake
    return true;
}

struct addrinfo *p;

// get sockaddr, IPv4 or IPv6:
void *get_in_addr(struct sockaddr *sa)
{
    if (sa->sa_family == AF_INET) {
        return &(((struct sockaddr_in*)sa)->sin_addr);
    }

    return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

int server_init() {
    int sockfd;
    struct addrinfo hints, *servinfo;
    int rv;
    int numbytes;
    struct sockaddr_storage their_addr;
    char buf[BUFFER_SIZE];
    socklen_t addr_len;
    char s[INET6_ADDRSTRLEN];

    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_INET; // set to AF_INET to use IPv4
    hints.ai_socktype = SOCK_DGRAM;
    hints.ai_flags = AI_PASSIVE; // use my IP

    if ((rv = getaddrinfo(NULL, PORT, &hints, &servinfo)) != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
        return 1;
    }

    // loop through all the results and bind to the first we can
    for(p = servinfo; p != NULL; p = p->ai_next) {
        if ((sockfd = socket(p->ai_family, p->ai_socktype,
                p->ai_protocol)) == -1) {
            perror("listener: socket");
            continue;
        }

        if (bind(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
            close(sockfd);
            perror("listener: bind");
            continue;
        }

        break;
    }

    if (p == NULL) {
        fprintf(stderr, "listener: failed to bind socket\n");
        return 2;
    }

    freeaddrinfo(servinfo);

    return sockfd;
}

char *recv_msg(int sockfd) {
    socklen_t addr_len;
    int numbytes;
    struct sockaddr_storage their_addr;
    char buf[BUFFER_SIZE];
    char s[INET6_ADDRSTRLEN];
    printf("listener: waiting to recvfrom...\n");

    addr_len = sizeof their_addr;
    if ((numbytes = recvfrom(sockfd, buf, BUFFER_SIZE-1 , 0,
        (struct sockaddr *)&their_addr, &addr_len)) == -1) {
        perror("recvfrom");
        exit(1);
    }

    printf("listener: got packet from %s\n",
        inet_ntop(their_addr.ss_family,
            get_in_addr((struct sockaddr *)&their_addr),
            s, sizeof s));
    printf("listener: packet is %d bytes long\n", numbytes);
    buf[numbytes] = '\0';
    printf("listener: packet contains \"%s\"\n", buf);

    return buf;
}

int client_init(char *server_name) {
    int sockfd;
    struct addrinfo hints, *servinfo, *p;
    int rv;
    int numbytes;

    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_INET; // set to AF_INET to use IPv4
    hints.ai_socktype = SOCK_DGRAM;

    if ((rv = getaddrinfo(server_name, PORT, &hints, &servinfo)) != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
        return 1;
    }

    // loop through all the results and make a socket
    for(p = servinfo; p != NULL; p = p->ai_next) {
        if ((sockfd = socket(p->ai_family, p->ai_socktype,
                p->ai_protocol)) == -1) {
            perror("talker: socket");
            continue;
        }

        break;
    }

    if (p == NULL) {
        fprintf(stderr, "talker: failed to create socket\n");
        return 2;
    }

    freeaddrinfo(servinfo);

    return sockfd;
}

int send_msg(int sockfd, char *msg) {
    int numbytes;
    if ((numbytes = sendto(sockfd, msg, strlen(msg), 0,
             p->ai_addr, p->ai_addrlen)) == -1) {
        perror("talker: sendto");
        exit(1);
    }

    // printf("talker: sent %d bytes to %s\n", numbytes, argv[1]);
    close(sockfd);

    return 0;
}

void close_socket(int sockfd) { close(sockfd); }
