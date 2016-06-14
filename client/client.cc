#include "client.h"

bool DEBUG = false;


void client(int port) {
    bool run = true;

    char buffer[BUFFER_SIZE];
    string command;
    vector<string> cmds;

    int sock_msg;
    struct hostent * host;
    
}