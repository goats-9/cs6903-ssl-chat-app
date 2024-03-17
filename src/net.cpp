#include "net.hpp"
#include "tui.hpp"
#include <bits/stdc++.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>
#define PORT_NUM 8080
#define BUFFER_SIZE 1024
using namespace std;

int serverSocket;
int clientSocket;

void start_server()
{
    serverSocket = socket(AF_INET, SOCK_STREAM, 0);

    struct sockaddr_in serverAddr;
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = INADDR_ANY;
    serverAddr.sin_port = htons(PORT_NUM);

    bind(serverSocket, (struct sockaddr *)&serverAddr, sizeof(serverAddr));
    listen(serverSocket, 5);

    int newSocket = accept(serverSocket, nullptr, nullptr);
    /* create a new thread for each connection*/
    char msg[BUFFER_SIZE];
    /*'msg' from received_message func*/
    recv(newSocket, msg, sizeof(msg), 0);
}

void start_client()
{
    clientSocket = socket(AF_INET, SOCK_STREAM, 0);
    struct sockaddr_in serverAddress;
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_addr.s_addr = INADDR_ANY;
    serverAddress.sin_port = htons(8080);

    connect(clientSocket, (struct sockaddr *)&serverAddress, sizeof(serverAddress));
    char *message = "hello I am client";
    /* 'message' is the msg from send_message func*/
    send(clientSocket, message, strlen(message), 0);
}
void server_stop()
{
    close(serverSocket);
}
void client_stop()
{
    close(clientSocket);
}