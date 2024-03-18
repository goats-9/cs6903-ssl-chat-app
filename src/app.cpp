#include "app.hpp"
#include "net.hpp"

ChatApp::ChatApp(UserRole role, string user, string hostname, uint16_t port)
{
    this->role = role;
    this->user = user;
    this->hostname = hostname;
    this->port = port;

    initscr();
    init_color();

    noecho();
    cbreak();
    keypad(stdscr, TRUE);
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
    if (role == SERVER)
    {
        thread server_thread(&ChatApp::run_server, this);
        this->network_thread = &server_thread;
    }
    else
    {
        thread client_thread(&ChatApp::run_client, this);
        this->network_thread = &client_thread;
    }
    run_tui();
    stop_network_thread();
}

void ChatApp::run_client()
{
    // connect to server
    //  handshake
    //  call message_received when message is received
    //  start_client();
    Client client(ip, Port, chat_window);
    client.start(hostname);
}

void ChatApp::run_server()
{
    // start server
    // accept connections
    // create new thread for each connection
    // start_server();
    Server server(ip, Port, chat_window);
    server.start();
}

void ChatApp::run_tui()
{
    do
    {
        create_window();
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

void ChatApp::stop_client()
{
    // stop the client
    // client_stop();
    Client client(ip, Port, chat_window);
    client.stop();
}

void ChatApp::stop_server()
{
    // stop the server
    // server_stop();
    Server server(ip, Port, chat_window);
    server.stop();
}
