#include "app.hpp"
#include "net.hpp"
#include <fstream>

extern ofstream log_file;

ChatApp::ChatApp(UserRole role, string user, string hostname, uint16_t port)
{
    this->role = role;
    this->user = user;
    this->hostname = hostname;
    this->port = port;

    log_file << "ChatApp::ChatApp -> role: " << role << ", user: " << user << ", hostname: " << hostname << ", port: " << port << endl;

    initscr();
    init_color();

    noecho();
    cbreak();
    keypad(stdscr, TRUE);
    nodelay(stdscr, TRUE);
    refresh();

    user = "user1"; // TODO: get user config file or cmdline arg
}

void ChatApp::create_window()
{
    if (chatWindow != NULL)
    {
        delete chatWindow;
    }
    int height, width;

    getmaxyx(stdscr, height, width);
    chatWindow = new ChatWindow(stdscr, &messages, height, width, 0, 0, user);

    wrefresh(stdscr);
}

void ChatApp::init_color()
{
    start_color();

    init_pair(1, COLOR_WHITE, COLOR_BLACK);
    init_pair(2, COLOR_GREEN, COLOR_BLACK);
    init_pair(3, COLOR_BLUE, COLOR_BLACK);
    init_pair(4, COLOR_BLACK, COLOR_WHITE);
}

ChatApp::~ChatApp()
{
    if (chatWindow != NULL)
        delete chatWindow;
    endwin();
}

void ChatApp::run()
{
    log_file << "running" << endl;
    if (role == SERVER)
    {
        log_file << "starting thread" << endl;
        network_thread = new thread(&ChatApp::run_server, this);
        input_handler_thread = new thread(&ChatApp::run_input_handler, this);
        log_file << "thread started" << endl;
    }
    else
    {
        network_thread = new thread(&ChatApp::run_client, this);
        input_handler_thread = new thread(&ChatApp::run_input_handler, this);
    }
    run_tui();
    log_file << "tui stopped" << endl;
    stop_network_thread();
}

void ChatApp::run_client()
{
    // connect to server
    //  handshake
    //  call message_received when message is received
    //  start_client();
    // Client client(ip, Port, chat_window);

    while (chatWindow == NULL)
    {
        usleep(1000);
    }
    client = new Client(ip, port, chatWindow);
    log_file << "ChatApp::run_client: " << (int)port << endl;
    client->start(hostname);
    log_file << "client stopped" << endl;
}

void ChatApp::run_server()
{
    // start server
    // accept connections
    // create new thread for each connection
    // start_server();

    log_file << "running server" << endl;
    // Server server(ip, Port, chat_window);
    while (chatWindow == NULL)
    {
        usleep(1000);
    }
    server = new Server(ip, port, chatWindow);
    log_file << "server starting" << endl;
    server->start();
    log_file << "server stopped" << endl;
}

void ChatApp::run_tui()
{
    log_file << "running tui" << endl;
    do
    {
        create_window();
        chatWindow->send_queue = &send_queue;
        chatWindow->recv_queue = &recv_queue;
        chatWindow->send_queue_mutex = &send_queue_mutex;
        chatWindow->recv_queue_mutex = &recv_queue_mutex;
    } while (chatWindow->run_forever());
}

void ChatApp::stop_network_thread()
{
    // stop the network thread
    if (role == SERVER)
    {
        ChatApp::stop_server();
    }
    else
    {
        ChatApp::stop_client();
    }
}

void ChatApp::run_input_handler() {
    while (true) {
        send_queue_mutex.lock();
        // log_file << "ChatApp::run_input_handler -> send_queue.size(): " << send_queue.size() << endl;
        if (!send_queue.empty()) {
            string message = send_queue.front();
            log_file << "ChatApp::run_input_handler -> got message: " << message << endl;
            send_queue.pop();
            send_queue_mutex.unlock();
            if (role == SERVER) {
                server->socketObj->send_msg(message);
            } else {
                client->socketObj->send_msg(message);
            }
        } else {
            send_queue_mutex.unlock();
        }
        usleep(50);
    }
}

void ChatApp::stop_client()
{
    // stop the client
    // client_stop();
}

void ChatApp::stop_server()
{
    // stop the server
    // server_stop();
}
