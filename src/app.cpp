#include "app.hpp"

ChatApp::ChatApp() {
    initscr();
    init_color();

    noecho();
    cbreak();
    keypad(stdscr, TRUE);
    refresh();

    user = "user1"; // TODO: get user config file or cmdline arg
}

void ChatApp::create_window() {
    if (chatWindow != NULL) {
        delete chatWindow;
    }
    int height, width;

    getmaxyx(stdscr, height, width);
    chatWindow = new ChatWindow(stdscr, &messages, height, width, 0, 0, user);
    wrefresh(stdscr);
}

void ChatApp::init_color(){
    start_color();

    init_pair(1, COLOR_WHITE, COLOR_BLACK);
    init_pair(2, COLOR_GREEN, COLOR_BLACK);
    init_pair(3, COLOR_BLUE, COLOR_BLACK);
    init_pair(4, COLOR_BLACK, COLOR_WHITE);
}

ChatApp::~ChatApp() {
    if (chatWindow != NULL)
        delete chatWindow;
    endwin();
}

// run network
// void ..... {
    // get other_user name
    // msg = recv_from_other_user();
    // chatWindow->message_received(user, msg);
// }

void ChatApp::run_tui() {
    do {
        create_window();
    } while (chatWindow->run_forever());
}
