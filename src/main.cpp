#include "app.hpp"

// argument parsing

int main() {
    ChatApp app(SERVER, "user1", "localhost", 8080);
    app.run_tui();
}
