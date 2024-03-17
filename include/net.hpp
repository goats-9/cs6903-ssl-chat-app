#ifndef SSL_CHAT_APP_NET_H
#define SSL_CHAT_APP_NET_H
int server_init();
int client_init();
void close_socket(int);
int send_msg(int, char *);
char *recv_msg(int);
int send_ctrl_msg(int, char *);
char *recv_ctrl_msg(int);
#endif
