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
    running = true;
    log_file << "ChatApp::run(): running" << endl;
    if (role == SERVER)
    {
        log_file << "ChatApp::run(): starting server thread" << endl;
        network_thread = new thread(&ChatApp::run_server, this);
        input_handler_thread = new thread(&ChatApp::run_input_handler, this);
        log_file << "ChatApp::run(): server thread started" << endl;
    }
    else
    {
        log_file << "ChatApp::run(): starting client thread" << endl;
        network_thread = new thread(&ChatApp::run_client, this);
        input_handler_thread = new thread(&ChatApp::run_input_handler, this);
        log_file << "ChatApp::run(): client thread started" << endl;
    }
    run_tui();
    log_file << "ChatApp::run(): tui stopped" << endl;
    running = false;
    stop_network_thread();
    network_thread->join();
    log_file << "ChatApp::run(): network thread stopped" << endl;
    input_handler_thread->join();
    log_file << "ChatApp::run(): input handler thread stopped" << endl;
}

void ChatApp::run_client()
{
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
    log_file << "running server" << endl;
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
    while (running) {
        send_queue_mutex.lock();
        // log_file << "ChatApp::run_input_handler -> send_queue.size(): " << send_queue.size() << endl;
        if (!send_queue.empty()) {
            if (role == SERVER && server->socketObj->running == false){
                send_queue_mutex.unlock();
                continue;
            }
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
        usleep(1000);
    }
}

void ChatApp::stop_client()
{
    client->socketObj->stop();
}

void ChatApp::stop_server()
{
    server->socketObj->stop();
}
