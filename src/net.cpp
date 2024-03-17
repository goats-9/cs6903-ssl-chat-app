#include "net.hpp"
#include "app.hpp"

using namespace std;

#define PORT_NUM 8080
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
