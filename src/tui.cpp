#include "tui.hpp"
#include <fstream>
#include <algorithm>

ofstream log_file("log.log");

MessageBox::MessageBox(WINDOW *parent, string sender, const string &msg, int width, int starty, int startx, bool sender_is_self) {
    if (sender_is_self)
        sender = "you";

    msg_box_width = width;
    msg_box_height = (msg.length()+sender.length()+2) / (width+1) + 1; // +2 for ": "

    msg_box = subwin(parent, msg_box_height, msg_box_width, starty, startx);

    if (sender_is_self){
        wattron(msg_box, COLOR_PAIR(2));
    } else {
        wattron(msg_box, A_BOLD);
    }
    wprintw(msg_box, "%s: ", sender.c_str());

    if (sender_is_self)
        wattron(msg_box, COLOR_PAIR(1));
    else
        wattroff(msg_box, A_BOLD);
    wprintw(msg_box, "%s", msg.c_str());
    wrefresh(msg_box);
}

MessageBox::~MessageBox() {
    delwin(msg_box);
}

ChatBox::ChatBox(WINDOW *parent, const vector<Message>& messages, int height, int width, int starty, int startx, const string &user, int *scroll_offset) {
    chat_box_height = height;
    chat_box_width = width;
    this->user = user;
    this->scroll_offset = scroll_offset;
    chat_box = subwin(parent, chat_box_height, chat_box_width, starty, startx);
    scrollok(chat_box, TRUE);
    box(chat_box, 0, 0);

    int y = 1;
    auto msgs = get_messages_to_show(messages);
    for (auto &msg: msgs){
        msg_boxes.push_back(MessageBox(chat_box, msg.sender, msg.message, chat_box_width-2, starty+y, startx+1, msg.sender == user));
        y += msg_boxes.back().msg_box_height + 1;
        wmove(chat_box, y-1, 1);
        if (msg == msgs.back()){
            wattron(chat_box, COLOR_PAIR(3));
            whline(chat_box, 0, chat_box_width-2);
            wattron(chat_box, COLOR_PAIR(1));
        } else {
            whline(chat_box, 0, chat_box_width-2);
        }
    }

    wrefresh(chat_box);
}

ChatBox::~ChatBox() {
    msg_boxes.clear();
    wclear(chat_box);
    delwin(chat_box);
}

vector<Message> ChatBox::get_messages_to_show(const vector<Message>& messages) {
    int y = 1;
    int i = messages.size()-*scroll_offset-1;
    log_file << "scroll_offset: " << scroll_offset << endl;
    vector<Message> msgs;
    for (; i>=0; i--){
        // add number of lines required to display the message + 1 for the line separator
        y += ((messages[i].message.length()+messages[i].sender.length()+2) / (chat_box_width+1) + 1) + 1;
        if (y > chat_box_height-1) {
            break;
        }
        msgs.push_back(messages[i]);
    }
    reverse(msgs.begin(), msgs.end());
    for (int i = messages.size()-*scroll_offset; i<messages.size(); i++){
        y += ((messages[i].message.length()+messages[i].sender.length()+2) / (chat_box_width+1) + 1) + 1;
        if (y > chat_box_height-1) {
            break;
        }
        (*scroll_offset)--;
        msgs.push_back(messages[i]);
    }
    return msgs;
}

InputBox::InputBox(WINDOW *parent, int height, int width, int starty, int startx) {
    input_box_height = height;
    input_box_width = width;
    input_box_border = subwin(parent, input_box_height, input_box_width, starty, startx);

    wattron(input_box_border, COLOR_PAIR(2));
    box(input_box_border, 0, 0);
    wattron(input_box_border, COLOR_PAIR(1));

    input_box = subwin(input_box_border, height-2, width-2, starty+1, startx+1);
    wmove(input_box, 0, 0);
    // wclrtoeol(input_box);
    wprintw(input_box, "Type here: ");
    wrefresh(input_box);
}

InputBox::~InputBox() {
    delwin(input_box);
}

void InputBox::clear_input() {
    wmove(input_box, 1, 0);
    wclrtoeol(input_box);
    wmove(input_box, 0, 0);
    wclrtoeol(input_box);
    wattron(input_box_border, COLOR_PAIR(2));
    box(input_box_border, 0, 0);
    wattron(input_box_border, COLOR_PAIR(1));
}

void InputBox::print_message(const string &message, bool clear) {
    if (clear)
        clear_input();

    int visible_char_len = (input_box_width-2)*(input_box_height-2);
    string visible_message;
    if (message.length() < visible_char_len){
        visible_message = message;
    } else {
        visible_message = message.substr(message.length()-visible_char_len, visible_char_len);
    }
    wprintw(input_box, "%s", visible_message.c_str());
    wrefresh(input_box);
}

pair<string, enum InputEvent> InputBox::get_input(string prefilled, enum InputMode mode) {
    // wmove(input_box, 0, 0);
    // clear_input();
    // wprintw(input_box, "%s", prefilled.c_str());

    print_message(prefilled);

    string input = prefilled;
    int ch;
    while (true) {
        if (mode == INSERT){
            if (input.length() == 0){
                // clear_input();
                // wprintw(input_box, "Type here: ");
                print_message("Type here: ");
            }
        }
        else {
            // clear_input();
            // wprintw(input_box, "Press ESC to insert");
            print_message("Press ESC to insert");
        }
        wrefresh(input_box);
        ch = getch();

        if (ch == 10) {
            if (mode != INSERT)
                continue;
            if (input.length() != 0)
                return make_pair(input, ENTER);
        } else if (ch == 27) {
            return make_pair(input, ESCAPE);
        } else if (ch == KEY_BACKSPACE) {
            if (input.length() > 0) {
                input.pop_back();
                // clear_input();
                // wprintw(input_box, "%s", input.c_str());
                print_message(input);
            }
            if (input.length() == 0){
                if (mode == INSERT)
                    // wprintw(input_box, "Type here: ");
                    print_message("Type here: ");
                else {
                    clear_input();
                    wattron(input_box, COLOR_PAIR(4));
                    // wprintw(input_box, "Press ESC to exit");
                    print_message("Press ESC to exit");
                    wattron(input_box, COLOR_PAIR(1));
                }
            }
        } else if (ch == 267) {
            if (mode != INSERT)
                continue;
            if (input.length() != 0)
                return make_pair(input, USER2_ENTER);
        } else if (ch == KEY_UP) {
            return make_pair(input, UP);
        } else if (ch == KEY_DOWN) {
            return make_pair(input, DOWN);
        } else if (ch == KEY_LEFT || ch == KEY_RIGHT) {
            // eat five star do nothing
        } else if (ch == 4) {
            return make_pair(input, KEY_EOF);
        } else if (ch == KEY_RESIZE) {
            return make_pair(input, WIN_RESIZE);
        } else {
            if (mode == INSERT){
                log_file << "input: " << ch << endl;
                input += ch;
                // clear_input();
                // wprintw(input_box, "%s", input.c_str());
                print_message(input);
            }
        }
    }
}

ChatWindow::ChatWindow(WINDOW *parent, vector <Message> *messages, int height, int width, int starty, int startx, const string &user, int input_box_height) {
    this->user = user;
    this->messages = messages;
    this->input_box_height = input_box_height;
    chat_window_height = height;
    chat_window_width = width;
    if (parent == NULL){
        chat_window = newwin(chat_window_height, chat_window_width, starty, startx);
    } else {
        chat_window = subwin(parent, chat_window_height, chat_window_width, starty, startx);
    }
    // box(chat_window, 0, 0);

    create_chat_box();

    wrefresh(chat_window);
}

void ChatWindow::create_chat_box() {
    if (chat_box != NULL) {
        delete chat_box;
    }
    chat_box = new ChatBox(chat_window, *messages, chat_window_height-input_box_height, chat_window_width, 0, 0, user, &scroll_offset);

    if (input_box != NULL) {
        delete input_box;
    }
    input_box = new InputBox(chat_window, input_box_height, chat_window_width, chat_window_height-input_box_height, 0);
}

ChatWindow::~ChatWindow() {
    clear();
    if (chat_box != NULL) {
        delete chat_box;
    }
    if (input_box != NULL) {
        delete input_box;
    }
    delwin(chat_window);
    endwin();
}

void ChatWindow::send_message(const string &message) {
    messages->push_back({messages->size(), user, message});
    create_chat_box();

    // network.send_message(user, message);
}

void ChatWindow::message_received(const string &user, const string &message) {
    messages->push_back({messages->size(), "user2", message});
    create_chat_box();
}

bool ChatWindow::run_forever() {
    InputMode mode = INSERT;
    while (true) {
        auto input = input_box->get_input(prefilled, mode);
        switch (input.second) {
            case ENTER:
                send_message(input.first);
                prefilled = "";
                break;
            case ESCAPE:
                if (is_scrolling){
                    is_scrolling = false;
                    scroll_offset = 0;
                    mode = INSERT;
                    create_chat_box();
                } else {
                    is_scrolling = true;
                    mode = NORMAL;
                    prefilled = input.first;
                }
                    break;
            case UP:
                if (is_scrolling){
                    scroll_offset++;
                    create_chat_box();
                } else {
                    prefilled = input.first;
                }
                break;
            case DOWN:
                if (is_scrolling){
                    scroll_offset--;
                    if (scroll_offset < 0)
                        scroll_offset = 0;
                    create_chat_box();
                } else {
                    prefilled = input.first;
                }
                break;
            case WIN_RESIZE:
                getmaxyx(stdscr, chat_window_height, chat_window_width);
                wresize(chat_window, chat_window_height, chat_window_width);
                create_chat_box();
                return true;
            case KEY_EOF:
                return false;
            case USER2_ENTER:
                message_received("user2", input.first);
                break;
            default:
                log_file << "Unknown input event: " << input.second << endl;
                break;
        }
    }
}


