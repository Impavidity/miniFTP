#include <string.h>
#include <iostream>
#include <pthread.h>
#include <sys/socket.h>
#include <netinet/in.h>

using namespace std;

enum SERVER_ERROR{
    STREAM_SOCKET_ERROR,
    BIND_SOCKET_ERROR,
    ACCEPT_ERROR,
    FORK_ERROR
};

#define BUFFER_SIZE 4096


void* server(void* listen_port);
void *new_service(void * sock);