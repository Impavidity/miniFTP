#include "client.h"


void client(int port) {
    bool run = true;

    char buffer[BUFFER_SIZE];
    string command;
    vector<string> cmds;

    int sock_msg;
    struct hostent * host;
    struct sockaddr_in server_msg;

    host = gethostbyname(server_name);
    if (!host) {
        cerr << "Get host by name failed" << endl;
        return;
    }

    sock_msg = socket(AF_INET, SOCK_STREAM, 0);
    if (sock_msg < 0) {
        cerr << "Open stream socket failed" << endl;
        return;
    }

    server_msg.sin_family = AF_INET;
    server_msg.sin_port = htons(port);
    memcpy((char*)&server_msg.sin_addr, host->h_addr_list[0], (size_t)host->h_length);

    if (connect(sock_msg, (struct sockaddr*)&server_msg, sizeof(server_msg)) < 0) {
        cerr << "Connect stream socket failed" << endl;
        return;
    }

    cerr << "Client in" << endl;
    if (read(sock_msg, buffer, sizeof(buffer)) < 0) {
        cerr << "Read failed" << endl;
    } else {
        cerr << buffer << endl;
    }

    while (true) {
        if (not run) {
            cerr << "End the client" << endl;
            return;
        }

        getline(cin, command);
        boost::split(cmds, command, boost::is_any_of(" \t"));
        if (cmds.size() < 1) continue;
        const char* cmd = cmds[0].c_str();
        if (cmds.size() == 1 && strcmp(cmd, CMD_QUIT) == 0) {
            run = false;
            continue;
        } 
        if (strcmp(cmd, CMD_LS) == 0) {
            cmd_list(sock_msg);
            continue;
        }
        if (cmds.size() == 1 && strcmp(cmd, CMD_HELP) == 0) {
            cmd_help(sock_msg);
            continue;
        }
        if (strcmp(cmd, CMD_PULL) == 0) {

        }
        if (strcmp(cmd, CMD_PUSH) == 0) {

        }
        if (strcmp(cmd, CMD_CD) == 0) {
            cmd_cd(sock_msg, cmds);
            continue;
        }
        if (strcmp(cmd, CMD_PWD) == 0) {
            cmd_pwd(sock_msg);
            continue;
        }
        cerr << "Command not found" << endl;

    }

}

void cmd_pwd(int sock) {
    char buffer[BUFFER_SIZE];

    if (write(sock, CMD_PWD, strlen(CMD_PWD)+1) < 0) {
        cerr << "Write failed" << endl;
        return;
    }
    if (read(sock, buffer, sizeof(buffer)) < 0) {
        cerr << "Read failed" << endl;
        return;
    }
    cerr << "Current Directory: " << buffer << endl;
}

void cmd_list(int sock) {
    char buffer[BUFFER_SIZE];

    write(sock, CMD_LS, sizeof(CMD_LS) + 1);
    if (read(sock, buffer, sizeof(buffer)) < 0) {
        cerr << "Read Failed" << endl;
        return;
    }
    cerr << buffer;
}

void cmd_help(int sock) {
    char buffer[BUFFER_SIZE];
    write(sock, CMD_HELP, sizeof(CMD_HELP) + 1);
    if (read(sock, buffer, sizeof(buffer)) < 0) {
        cerr << "Read Failed" << endl;
        return;
    }
    cerr << buffer << endl;
}

void cmd_cd(int sock, vector<string>& cmd) {
    char buffer[BUFFER_SIZE];

    if (cmd.size() > 2) {
        cerr << "There are too many arguments" << endl;
        return;
    }

    auto dir = cmd[1];
    if (write(sock, CMD_CD, strlen(CMD_CD)+1) < 0) {
        cerr << "Write failed" << endl;
        return;
    }
    cerr << cmd[1].c_str() << endl;

    if (read(sock, buffer, BUFFER_SIZE) < 0) {
        cerr << "Read ok failed" << endl;
        return;
    }
    if (strcmp(buffer, OK) != 0) {
        cerr << "Server not ok" << endl;
        return;
    }

    if (write(sock, dir.c_str(), dir.size()+1) < 0) {
        cerr << " Write Failed" << endl;
        return;
    }

    if (read(sock, buffer, BUFFER_SIZE) < 0) {
        cerr << "Read Failed" << endl;
        return;
    }

    cerr << "Current Directory : " << buffer << endl;
}


void cmd_pull(int sock, vector<string>& cmd) {
    char buffer[BUFFER_SIZE];

    if (cmd.size() > 2) {
        cerr << "There are too many arguments" << endl;
        return;
    }
    if (cmd.size() == 1) {
        cerr << "There isn't any file name" << endl;
        return;
    }
    if (send(sock, CMD_PULL, strlen(CMD_PULL)+1, 0) < 0) {
        cerr << "Write pull failed" << endl;
        return;
    }
    if (read(sock, buffer, BUFFER_SIZE) < 0) {
        cerr << "Read ok failed" << endl;
        return;
    }
    if (strcmp(buffer, OK) != 0) {
        cerr << "Server is not ok" << endl;
        return;
    }
    if (read(sock, buffer, BUFFER_SIZE) < 0) {
        cerr << "Read failed"<< endl;
        return;
    }
    if (strcmp(buffer, ERR_NULL) == 0) {
        cerr << "pull null" << endl;
        return;
    }
    if (write(sock, OK, strlen(OK)+1) < 0) {
        cerr << "Write OK failed" << endl;
        return;
    }

    long fsize;
    sscanf(buffer, "%ld", &fsize);
    cerr << "File Size : " << fsize << endl;

    FILE* file = fopen(cmd[1].c_str(), "w");
    long size = 0;
    char file_buffer[BUFFER_SIZE];
    while (fsize > 0) {
        bzero(file_buffer, BUFFER_SIZE);
        cerr << fsize << " left" << endl;
        size = recv(sock, file_buffer, BUFFER_SIZE, 0);
        if (size <= 0) {
            cerr << "Read failed" << endl;
            fclose(file);
            return;
        }
        fwrite(file_buffer, 1, size, file);
        fsize -= size;
        if (fsize > 0) {
            if (write(sock, OK, strlen(OK)+1) < 0) {
                cerr << "Write OK failed" << endl;
                return;
            }
        }
    }

    fclose(file);
    cerr << "End PULL" << endl;
}

void cmd_push(int sock, vector<string> & cmd) {
    char buffer[BUFFER_SIZE];

    if (write(sock ,CMD_PUSH, strlen(CMD_PUSH)+1) < 0) {
        cerr << "Write push failed" << endl;
        return;
    }
    if (cmd.size() > 2) {
        cerr << "There are too many arguments" << endl;
        return;
    }

    auto file_name = cmd[1];

    if (read(sock, buffer, BUFFER_SIZE) < 0) {
        cerr << "Read Failed" << endl;
        return;
    }

    if (strcmp(buffer, OK) != 0) {
        cerr << "server is not OK" << endl;
        return;
    }

    if (write(sock, file_name.c_str(), file_name.size()+1) < 0){
        cerr << "Write file name failed" << endl;
        return;
    }

    if (read(sock, buffer, BUFFER_SIZE) < 0) {
        cerr << "Read Failed" << endl;
        return;
    }
    if (strcmp(buffer, ERR_NULL) == 0) {
        cerr << "Push null, server refuse the push" << endl;
        return;
    }

    FILE* file = fopen(file_name.c_str(), "r");
    if (!file) {
        cerr << "No such file " << file_name.c_str()<< endl;
        return;
    }

    fseek(file, 0, SEEK_END);
    long fsize = ftell(file);
    stringstream fstrsize;
    fstrsize << fsize;
    fseek(file, 0, SEEK_SET);

    if (write(sock, fstrsize.str().c_str(), fstrsize.str().size()+1) < 0) {
        cerr << "Write file size failed" << endl;
        return;
    }
    if (read(sock, buffer, BUFFER_SIZE) < 0) {
        cerr << "Read failed" << endl;
        return;
    }
    if (strcmp(buffer, OK) != 0) {
        cerr << "Server not OK" << endl;
        return;
    } else {
        cerr << "Server is OK" << endl;
    }

    long size;
    while (fsize > 0) {
        bzero(buffer, BUFFER_SIZE);
        size = fread(buffer, 1, BUFFER_SIZE-1, file);
        stringstream content;
        content<<buffer;
        if (send(sock, content.str().c_str(), content.str().size(), 0) < 0) {
            cerr << "Write file failed" << endl;
            return;
        }
        fsize -= size;

        if (fsize > 0) {
            if (read(sock, buffer, BUFFER_SIZE) < 0) {
                cerr << "Read Failed" << endl;
                return;
            }
            if (strcmp(buffer, OK) != 0) {
                cerr << "Server not OK" << endl;
                return;
            }

        }
    }

    fclose(file);
    cerr << "End push" << endl;
}