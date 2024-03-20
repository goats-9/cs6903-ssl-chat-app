#include "app.hpp"
#include "net.hpp"
#include <bits/stdc++.h>
using namespace std;
// argument parsing

ofstream log_file;

int main(int argc, char *argv[])
{
    if (argc < 2)
    {
        cout << "Usage: " << argv[0] << " [-s/-c] <serverhostname>" << endl;
        return 1;
    }

    if (strcmp(argv[1], "-s") == 0)
    {
        log_file.open("server.log");
        ChatApp app(SERVER, "bob", "localhost", 8080);
        app.run();
    }
    else if (strcmp(argv[1], "-c") == 0)
    {
        log_file.open("client.log");
        ChatApp app(CLIENT, "alice", "localhost", 8080);
        app.run();
    }
    else
    {
        cout << "choose client or server using '-c' or '-s'" << endl;
    }
}
