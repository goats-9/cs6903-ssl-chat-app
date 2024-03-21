#include "net.hpp"
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <fstream>

#define PORT_NUM 8080
#define PORT "8080"
#define BUFFER_SIZE 1024

extern ofstream log_file;

// get sockaddr, IPv4 or IPv6:
void *get_in_addr(struct sockaddr *sa)
{
    if (sa->sa_family == AF_INET)
    {
        return &(((struct sockaddr_in *)sa)->sin_addr);
    }

    return &(((struct sockaddr_in6 *)sa)->sin6_addr);
}

string recv_msg_from(int skt, struct sockaddr_in &addr){
    char buffer[BUFFER_SIZE];
    socklen_t addr_len = sizeof(addr);
    ssize_t numbytes;
    sockaddr_in addr2 = {0};

    do {
        if ((numbytes=recvfrom(skt, buffer, BUFFER_SIZE, 0, (struct sockaddr *)&addr2, &addr_len)) == -1)
        {
            perror("socket: receive_msg");
            exit(1);
        }
        if (numbytes == 0){
            return "";
        }
        if (addr.sin_addr.s_addr == 0 && addr.sin_port == 0){
            addr = addr2;
            break;
        }
    } while (addr.sin_addr.s_addr != addr2.sin_addr.s_addr || addr.sin_port != addr2.sin_port);

    buffer[numbytes] = '\0';

    return string(buffer);
}

void send_msg_to(int skt, struct sockaddr_in addr, string msg){
    if (sendto(skt, msg.c_str(), msg.length(), 0, (struct sockaddr*)&addr, sizeof(addr)) == -1)
    {
        perror("socket: send_msg");
        exit(1);
    }
}

Socket::Socket(ChatWindow *chat_window, int skt)
{
    this->chat_window = chat_window;
    if (skt == -1)
    {
        if (this->skt = socket(AF_INET, SOCK_DGRAM, 0))
        {
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
    log_file << "Receiving message" << endl;
    socklen_t addr_len = sizeof(addr);
    sockaddr_in addr = {0};
    ssize_t numbytes;
    do {
        if ((numbytes=recvfrom(skt, buffer, BUFFER_SIZE, 0, (struct sockaddr *)&addr, &addr_len)) == -1)
        {
            perror("socket: receive_msg");
            exit(1);
        }
        if (numbytes == 0){
            return "";
        }
    } while (addr.sin_addr.s_addr != this->addr.sin_addr.s_addr || addr.sin_port != this->addr.sin_port);

    buffer[numbytes] = '\0';

    log_file << "Received message: " << string(buffer) << endl;
    return string(buffer);
}

string Socket::receive_ctl_msg()
{
    string recv_msg = Socket::receive_msg();
    // Send ACK
    send_msg("ACK");
}

void Socket::send_msg(string msg)
{
    log_file << "Sending message: " << msg << endl;
    if (sendto(skt, msg.c_str(), msg.length(), 0, (struct sockaddr*)&addr, sizeof(addr)) == -1)
    {
        perror("socket: send_msg");
        exit(1);
    }
}

void Socket::send_ctl_msg(string msg)
{
    Socket::send_msg(msg);
    // Wait for ACK
    while (Socket::receive_msg() != "ACK")
        ;
    return;
}

void Socket::run()
{
    running = true;
    while (running)
    {
        string msg = receive_msg();
        if (msg == "")
        {
            continue;
        }
        log_file << "Socket::run(): received message: " << msg << endl;
        chat_window->recv_queue_mutex->lock();
        chat_window->recv_queue->push(make_pair(user, msg));
        chat_window->recv_queue_mutex->unlock();
        log_file << "Socket::run(): pushed message to recv_queue" << endl;
        // chat_window->message_received(user, msg);
    }
}

void Socket::stop()
{
    running = false;
    shutdown(skt, SHUT_RDWR);
}

Server::Server(uint32_t ip, int port, ChatWindow *chat_window)
{
    this->chat_window = chat_window;
    this->ip = ip;
    this->port = port;
    log_file << "Creating socket" << endl;
    skt = socket(AF_INET, SOCK_DGRAM, 0);
    if (skt == -1)
    {
        perror("socket: socket");
        exit(1);
    }
    log_file << "Socket created" << endl;
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

    if ((rv = getaddrinfo(NULL, PORT, &hints, &this->servinfo)) != 0)
    {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
        exit(1);
    }

    handle_client();
}

void Server::handle_client()
{
    struct addrinfo *p;
    // loop through all the results and bind to the first we can
    for (p = this->servinfo; p != NULL; p = p->ai_next)
    {
        if ((this->skt = socket(p->ai_family, p->ai_socktype,
                                p->ai_protocol)) == -1)
        {
            perror("listener: socket");
            continue;
        }

        if (bind(this->skt, p->ai_addr, p->ai_addrlen) == -1)
        {
            close(this->skt);
            perror("listener: bind");
            continue;
        }

        server_addr = *(sockaddr_in*)(p->ai_addr);

        break;
    }

    if (p == NULL)
    {
        fprintf(stderr, "listener: failed to bind socket\n");
        exit(2);
    }

    freeaddrinfo(this->servinfo);
    socketObj = new Socket(chat_window, this->skt);
    if (!Server::handshake()){
        return;
    }
    log_file << "Handshake done" << endl;
    // socketObj->user = "client";
    socketObj->addr = client_addr;
    socketObj->chat_window = chat_window;
    socketObj->run();
}

// void Server::start() {

//     server_addr.sin_family = AF_INET;
//     server_addr.sin_port = htons(port);
//     server_addr.sin_addr.s_addr = INADDR_ANY;

//     if (bind(skt, (struct sockaddr *)&server_addr, sizeof(server_addr)) == -1)
//     {
//         perror("socket: bind");
//         exit(1);
//     }

//     socketObj = new Socket(chat_window, skt);

//     Server::handshake();
// }

bool Server::handshake()
{
    char buf[BUFFER_SIZE];
    ssize_t numbytes;
    struct sockaddr_in client_addr;
    socklen_t addr_len = sizeof(client_addr);

    log_file << "Server::handshake(): server addr: " << inet_ntoa(server_addr.sin_addr) << ":" << ntohs(server_addr.sin_port) << endl;

    // receive chat_START_SSL message
    string msg = recv_msg_from(skt, client_addr);
    log_file << "Server::handshake(): start_SSL_CHAT received: " << msg << endl;
    if (msg != "chat_START_SSL")
    {
        log_file << "Server::handshake(): Invalid message received: " << msg << endl;
        return false;
    }

    // send chat_START_SSL_ACK message
    send_msg_to(skt, client_addr, "chat_START_SSL_ACK");
    log_file << "Server::handshake(): chat_START_SSL_ACK sent" << endl;

    // receive username
    string username = recv_msg_from(skt, client_addr);
    log_file << "Server::handshake(): username received: " << username << endl;
    socketObj->user = username;

    // send username_ACK
    send_msg_to(skt, client_addr, "username_ACK");
    log_file << "Server::handshake(): username_ACK sent" << endl;

    // send our username
    send_msg_to(skt, client_addr, chat_window->user);
    log_file << "Server::handshake(): username sent: " << chat_window->user << endl;

    // receive username_ACK
    msg = recv_msg_from(skt, client_addr);
    if (msg != "username_ACK")
    {
        log_file << "Server::handshake(): Invalid message received: " << msg << endl;
        return false;
    }
    log_file << "Server::handshake(): username_ACK received" << endl;

    log_file << "Server::handshake(): client addr: " << inet_ntoa(client_addr.sin_addr) << " " << ntohs(client_addr.sin_port) << endl;

    this->client_addr = client_addr;
    return true;
}

void Server::stop()
{
    socketObj->send_msg("exit");
    socketObj->stop();
    close(this->skt);
}

Client::Client(uint32_t ip, int port, ChatWindow *chat_window)
{
    this->chat_window = chat_window;
    this->ip = ip;
    log_file << "Client: " << ip << ":" << port << endl;
    this->port = port;
    skt = socket(AF_INET, SOCK_DGRAM, 0);
}

void Client::start(string server_name)
{
    struct addrinfo hints, *p;
    int rv;

    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_INET; // set to AF_INET to use IPv4
    hints.ai_socktype = SOCK_DGRAM;

    if ((rv = getaddrinfo(server_name.c_str(), PORT, &hints, &this->servinfo)) != 0)
    {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
        exit(1);
    }

    // loop through all the results and make a socket
    for (p = this->servinfo; p != NULL; p = p->ai_next)
    {
        if ((this->skt = socket(p->ai_family, p->ai_socktype,
                                p->ai_protocol)) == -1)
        {
            perror("talker: socket");
            continue;
        };

        server_addr = *(sockaddr_in *)(p->ai_addr);

        break;
    }

    if (p == NULL)
    {
        fprintf(stderr, "talker: failed to create socket\n");
        exit(2);
    }

    freeaddrinfo(servinfo);
    socketObj = new Socket(chat_window, this->skt);
    if (!Client::handshake())
        return;
    log_file << "Handshake done" << endl;
    // socketObj->user = "Server";
    socketObj->addr = server_addr;
    socketObj->chat_window = chat_window;
    socketObj->run();
}

// void Client::start(string servername) {
//     server_addr.sin_family = AF_INET;
//     server_addr.sin_port = htons(port);
//     log_file << "Client::start: " << servername << ":" << port << endl;
//     server_addr.sin_addr.s_addr = inet_addr(servername.c_str());

//     Client::handshake();
// }

bool Client::handshake()
{
    ssize_t numbytes;
    log_file << "Client::handshake(): " << inet_ntoa(server_addr.sin_addr) << " " << ntohs(server_addr.sin_port) << endl;
    send_msg_to(skt, server_addr, "chat_START_SSL");
    log_file << "Client::handshake(): chat_START_SSL sent" << endl;

    string msg = recv_msg_from(skt, server_addr);
    if (msg != "chat_START_SSL_ACK")
    {
        log_file << "Client::handshake(): Invalid message received: " << msg << endl;
        return false;
    }
    log_file << "Client::handshake(): chat_START_SSL_ACK received" << endl;

    send_msg_to(skt, server_addr, chat_window->user);
    log_file << "Client::handshake(): username sent: " << chat_window->user << endl;

    msg = recv_msg_from(skt, server_addr);
    if (msg != "username_ACK")
    {
        log_file << "Client::handshake(): Invalid message received: " << msg << endl;
        return false;
    }
    log_file << "Client::handshake(): username_ACK received" << endl;

    sockaddr_in dummy = {0};
    socketObj->user = recv_msg_from(skt, dummy);
    log_file << "Client::handshake(): username received: " << socketObj->user << endl;

    send_msg_to(skt, server_addr, "username_ACK");
    log_file << "Client::handshake(): username_ACK sent" << endl;

    return true;
}

void Client::stop()
{
    socketObj->send_msg("exit");
    socketObj->stop();
    close(this->skt);
}
