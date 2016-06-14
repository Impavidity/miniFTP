#include "client.h"

int main(int argc, char * argv[]) {
    int port;
    sscanf(argv[1], "%d", &port);
    cerr << "port" << port << endl;
    client(port);
    return 0;
}