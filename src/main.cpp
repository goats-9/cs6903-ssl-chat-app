#include "app.hpp"
#include "net.hpp"
#include <bits/stdc++.h>
using namespace std;
// argument parsing

int main(int argc, char *argv[])
{
    if (argc < 2)
    {
        cout << "Usage: " << argv[0] << " [-s/-c] <serverhostname>" << endl;
        return 1;
    }

    if (strcmp(argv[1], "-s") == 0)
    {
        ChatApp app(SERVER, "bob", "localhost", 8080);
        app.run_tui();
    }
    else if (strcmp(argv[1], "-c") == 0)
    {
        ChatApp app(CLIENT, "alice", "localhost", 8080);
        app.run_tui();
    }
    else
    {
        cout << "choose client or server using '-c' or '-s'" << endl;
    }
}
