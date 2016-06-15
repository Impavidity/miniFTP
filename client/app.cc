#include "client.h"

int main(int argc, char * argv[]) {
    int port;
    if (argc != 2) {
        cerr << "You need to input the port number" << endl;
        return 1;
    }
    sscanf(argv[1], "%d", &port);
    cerr << "port " << port << endl;
    client(port);
    return 0;
}