#ifndef MINIFTP_CLIENT_H
#define MINIFTP_CLIENT_H

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <netdb.h>
#include <stdio.h>
#include <string.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <unistd.h>
#include <vector>
#include <string>
#include <sstream>
#include <fstream>
#include <iostream>
#include <boost/algorithm/string.hpp>
using namespace std;


#define LISTEN_PORT 12345
#define BUFFER_SIZE 4096
#define FILE_BUFFER_SIZE 8
#define COMMAND_BUFFER 1024

#define CMD_QUIT "quit"
#define CMD_HELP "help"
#define CMD_LS "ls"
#define CMD_PULL "pull"
#define CMD_PUSH "push"
#define CMD_END "end"
#define CMD_CD "cd"
#define CMD_PWD "pwd"

#define SERVER_REFUSE "refuse"
#define SERVER_ACCEPT "accept"

#define OK "ok"

#define ERR_NULL "NULL"

// Message DEFINITION
const char server_name[] = "127.0.0.1";

void client(int port);
void cmd_list(int sock);
void cmd_help(int sock);
void cmd_cd(int sock, vector<string>& cmd);
void cmd_pull(int sock, vector<string>& cmd);
void cmd_push(int sock, vector<string>& cmd);
void cmd_pwd(int sock);


#endif //MINIFTP_CLIENT_H