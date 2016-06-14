#include <string.h>
#include <iostream>
#include <pthread.h>
#include <vector>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <dirent.h>
#include <unistd.h>

using namespace std;

enum SERVER_ERROR{
    STREAM_SOCKET_ERROR,
    BIND_SOCKET_ERROR,
    ACCEPT_ERROR,
    FORK_ERROR
};

#define BUFFER_SIZE 4096
#define ROOT "ftp/"

// COMMAND DEFINITION
#define CMD_HELP "help"
#define CMD_LS "ls"

#define CLIENT_PULL "pull"
#define CLIENT_PUSH "push"
#define CLIENT_END "end"
#define CLIENT_CD "cd"
#define CLIENT_PWD "pwd"

#define OK "ok"

class node{
public:
    string name;
    string path;
    node * parent;
    vector<node> child;
    bool isDir;

    node(string name) {
        this->name = this->path = name;
        parent = NULL;
        isDir = true;
    }

    node(string name, node& parent){
        this->parent = &parent;
        this->name = name;
        this->path = parent.path + name;
        isDir = false;
    }

    node(string name, node& parent, bool isDir) {
        this->parent = &parent;
        this->name = name;
        this->isDir = isDir;
        this->path = parent.path + name;
        if (isDir) this->path += "/";
    }
};

string list_dir(node*& curr);
void* server(void* listen_port);
void *new_service(void * sock);

void cmd_help(int sock);
void cmd_list(int sock,node* cur);
void client_cd(int sock, node*& curr);
void client_pwd(int sock, node*& curr);
void client_pull(int sock, node* curr);
void client_push(int sock, node* curr);

void traverse_dir(node& n, const char * dir);
