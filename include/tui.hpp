#ifndef SSL_CHAT_APP_TUI_H
#define SSL_CHAT_APP_TUI_H

#include <ncurses.h>
#include <string>
#include <vector>
#include <queue>
#include <mutex>

using namespace std;


enum InputEvent {
    UP,
    DOWN,
    LEFT,
    RIGHT,
    ENTER,
    ESCAPE,
    WIN_RESIZE,
    KEY_EOF,
    USER2_ENTER,
    NOTHING,
};

enum InputMode {
    NORMAL,
    INSERT,
    COMMAND,
};

typedef struct message_t {
    ulong id;
    string sender;
    string message;

    message_t(ulong id, const string &sender, const string &message) : id(id), sender(sender), message(message) {}

    bool operator==(const struct message_t &other) const {
        return id == other.id;
    }
} Message;

class MessageBox {
    WINDOW *msg_box;

    public:
    int msg_box_height;
    int msg_box_width;

    MessageBox(WINDOW *parent, string sender, const string &msg, int width, int starty, int startx, bool sender_is_self);
    ~MessageBox();
};

class ChatBox {
    WINDOW *chat_box;
    int chat_box_height;
    int chat_box_width;
    int *scroll_offset;
    string user;

    vector<MessageBox> msg_boxes;

    vector<Message> get_messages_to_show(const vector<Message>& messages);

    public:
    ChatBox(WINDOW *parent, const vector<Message>& messages, int height, int width, int starty, int startx, const string &user, int *scroll_offset);
    ~ChatBox();
};

class InputBox {
    WINDOW *input_box_border;
    WINDOW *input_box;
    int input_box_height;
    int input_box_width;

    public:
    InputBox(WINDOW *parent, int height, int width, int starty, int startx);
    ~InputBox();

    // return the input string and a boolean
    // If boolean is true then enter was pressed
    // If boolean is false then escape was pressed
    pair<string, InputEvent> get_input(string prefilled="", InputMode mode=INSERT, bool refresh=true);
    void clear_input();
    void print_message(const string &message, bool clear=true);
};

class ChatWindow {
    WINDOW *chat_window=NULL;
    InputBox *input_box=NULL;
    ChatBox *chat_box=NULL;
    vector <Message> *messages;
    int chat_window_height;
    int chat_window_width;
    int input_box_height;
    int scroll_offset = 0;
    bool is_scrolling = false;
    string prefilled="";

    mutex tui_lock;

    void create_chat_box();

    public:
    string user;
    queue<string> *send_queue;
    queue<pair<string, string>> *recv_queue;
    mutex *send_queue_mutex;
    mutex *recv_queue_mutex;
    ChatWindow(WINDOW *parent, vector <Message> *messages, int height, int width, int starty, int startx, const string &user, int input_box_height=4);
    ~ChatWindow();

    void message_received(const string &user, const string &message);
    void send_message(const string &message);
    void *callback(string message);
    bool run_forever();
};

#endif //SSL_CHAT_APP_TUI_H
