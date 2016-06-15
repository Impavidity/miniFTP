//
// Created by stephen on 16-6-5.
//

#include "server.h"

bool run = true;
SERVER_ERROR ERR;

// MESSAGE DEFINITION
string welcome = "Welcome!\n";

string hls = "ls: list files under the current directory\n";
string hcd = "cd [dir name]: change the directory.\n";
string hpull = "pull [file name]: pull file from the ftp server.\n";
string hpush = "push [file name]: push file to the ftp server\n";
string help =
        "Help\n"
        + hls
        + hcd
        + hpull
        + hpush;

vector<string> fprotected = {"app.cpp", "CMakeLists.txt", "dir.cpp", "server.cpp", "server.h", "shell.sh", "server"};

void *server(void *listen_port) {

    int port = *((int *) (listen_port));
    int sock_msg;
    struct sockaddr_in server_msg;

    sock_msg = socket(AF_INET, SOCK_STREAM, 0);
    if (sock_msg < 0) {
        fprintf(stderr, "Open stream socket failed.\n");
        ERR = STREAM_SOCKET_ERROR;
        return NULL;
    }

    //    set the attributes of the server message
    server_msg.sin_family = AF_INET;
    server_msg.sin_addr.s_addr = INADDR_ANY;
    server_msg.sin_port = htons(port);
    //    bind the port for each socket
    if (bind(sock_msg, (struct sockaddr *) &server_msg, sizeof(server_msg)) < 0) {
        fprintf(stderr, "Bind stream socket failed.\n");
        ERR = BIND_SOCKET_ERROR;
        return NULL;
    }

    fprintf(stdout, "Socket port: %d\n", ntohs(server_msg.sin_port));
    listen(sock_msg, 2);

    while (run) {
        int msg_sock;
        int msg_len;
        msg_len = sizeof(struct sockaddr_in);
        msg_sock = accept(sock_msg, (struct sockaddr *) 0, (socklen_t *) &msg_len);

        if (msg_sock == -1) {
            fprintf(stderr, "Accept failed.\n");
            ERR = ACCEPT_ERROR;
        } else {
            pthread_t thread;
            pthread_create(&thread, NULL, new_service, (void *) &msg_sock);
        }
    }
}

void *new_service(void *sock) {
    int msg_sock = *((int *) sock);
    char buffer[BUFFER_SIZE];

    node root = node(ROOT);
    traverse_dir(root, ROOT);
    node *curr = &root;

    fprintf(stdout, "Connection success.\n");
    if (write(msg_sock, welcome.c_str(), welcome.size() + 1) < 0) {
        fprintf(stderr, "Write failed.\n");
    }
    fprintf(stdout, "Send message success.\n");

    int snd_size = 0, rcv_size = 0;
    socklen_t optlen = sizeof(snd_size);

    int err;
    err = getsockopt(*(int *) sock, SOL_SOCKET, SO_SNDBUF, &snd_size, &optlen);
    if (err < 0) {
        fprintf(stderr, "Error get send buffer size.\n");
    } else {
        fprintf(stdout, "Send buffer size: %d\n", snd_size);
    }
    err = getsockopt(*(int *) sock, SOL_SOCKET, SO_RCVBUF, &rcv_size, &optlen);
    if (err < 0) {
        fprintf(stderr, "Error get rcv buffer size.\n");
    } else {
        fprintf(stdout, "Send rcv size: %d\n", rcv_size);
    }


    while (run) {
        auto read_flag = read(msg_sock, buffer, sizeof(buffer));

        if (read_flag < 0) {
            fprintf(stderr, "Reading from client failed.\n");
            close(msg_sock);
            break;
        } else if (read_flag == 0) {
            fprintf(stdout, "Connection ended.\n");
            close(msg_sock);
        } else {
            if (strcmp(buffer, CMD_HELP) == 0) {
                cmd_help(msg_sock);
            } else if (strcmp(buffer, CMD_LS) == 0) {
                cmd_list(msg_sock, curr);
            } else if (strcmp(buffer, CLIENT_PULL) == 0) {
                client_pull(msg_sock, curr);
            } else if (strcmp(buffer, CLIENT_PUSH) == 0) {
                client_push(msg_sock, curr);
            } else if (strcmp(buffer, CLIENT_CD) == 0) {
                client_cd(msg_sock, curr);
            } else if (strcmp(buffer, CLIENT_PWD) == 0) {
                client_pwd(msg_sock, curr);
            } else {
                fprintf(stdout, "Command not found: %s.\n", buffer);
            }
        }
    }
}

void cmd_help(int sock) {
    fprintf(stdout, "Command help.\n");
    if (write(sock, help.c_str(), help.size() + 1) < 0) {
        fprintf(stderr, "Write failed.\n");
    }
    fprintf(stdout, "Send help msg success.\n");
}

void cmd_list(int sock, node *cur) {
    fprintf(stdout, "Command list.\n");
    auto list = list_dir(cur);
    if (write(sock, list.c_str(), strlen(list.c_str()) + 1) < 0) {
        fprintf(stderr, "Write failed.\n");
    }
    fprintf(stdout, "Send ls msg success.\n");
}

void client_pwd(int sock, node *&curr) {
    fprintf(stdout, "Command pwd.\n");
    if (write(sock, curr->name.c_str(), curr->name.size() + 1) < 0) {
        fprintf(stderr, "Write failed.\n");
        return;
    }
    fprintf(stdout, "Send pwd msg success.\n");
}

void client_cd(int sock, node *&curr) {
    char buffer[BUFFER_SIZE];
    if (send(sock, OK, strlen(OK) + 1, 0) < 0) {
        fprintf(stderr, "Write ok failed.\n");
        return;
    }
    fprintf(stdout, "Command cd.\n");

    auto read_flag = read(sock, buffer, sizeof(buffer));
    if (read_flag < 0) {
        fprintf(stderr, "Read failed.\n");
        return;
    } else if (read_flag == 0) {
        fprintf(stderr, "Connection ended.\n");
        return;
    } else {
        stringstream ss;
        ss << buffer;

        if (ss.str() == ".") {
            return;
        } else if (ss.str() == "..") {
            if (curr->parent == NULL) {
                sprintf(buffer, "%s\n", "Cannot change directory.");
                if (write(sock, buffer, strlen(buffer) + 1) < 0) {
                    fprintf(stderr, "Write failed.\n");
                    return;
                }
            } else {
                curr = curr->parent;
                sprintf(buffer, "%s\n", ss.str().c_str());
                if (write(sock, buffer, strlen(buffer) + 1) < 0) {
                    fprintf(stderr, "Write failed.\n");
                }
            }
        } else {
            auto &v = curr->child;
            for (auto &item : v) {
                if (item.name == ss.str()) {
                    curr = &item;
                    if (write(sock, curr->name.c_str(), curr->name.size() + 1) < 0) {
                        fprintf(stderr, "Write failed.\n");
                    }
                    return;
                }
            }
            sprintf(buffer, "No such directory: %s.\n", ss.str().c_str());
            if (write(sock, buffer, strlen(buffer) + 1) < 0) {
                fprintf(stderr, "Write failed.\n");
            }
        }
    }
}

void client_pull(int sock, node *curr) {
    char buffer[BUFFER_SIZE];
    fprintf(stdout, "Client pull files.\n");

    if (write(sock, OK, strlen(OK) + 1) < 0) {
        perror("Write ok failed.\n");
        return;
    }

    if (read(sock, buffer, BUFFER_SIZE) < 0) {
        fprintf(stderr, "Read failed.\n");
        return;
    }
    fprintf(stdout, "Get file name: %s\n", buffer);

    stringstream ss;
    ss << buffer;
    string file_name = curr->path + ss.str();

    FILE *file = fopen(file_name.c_str(), "r");
    if (!file) {
        fprintf(stderr, "No such file: %s\n", file_name.c_str());
        if (write(sock, ERR_NULL, strlen(ERR_NULL) + 1) < 0) {
            fprintf(stderr, "Write NULL failed.\n");
        }
        return;
    }

    fseek(file, 0, SEEK_END);
    long fsize = ftell(file);
    fprintf(stdout, "File Size: %ld\n", fsize);
    stringstream tmp;
    tmp << fsize;
    fseek(file, 0, SEEK_SET);

    if (write(sock, tmp.str().c_str(), tmp.str().size() + 1) < 0) {
        fprintf(stderr, "Write file size failed.\n");
        return;
    }

    if (read(sock, buffer, BUFFER_SIZE) < 0) {
        fprintf(stderr, "Read failed.\n");
        return;
    }
    if (strcmp(buffer, OK) != 0) {
        fprintf(stderr, "Client not ok.\n");
        return;
    }

    long size;
    char file_buffer[BUFFER_SIZE];
    while (fsize > 0) {
        bzero(file_buffer, BUFFER_SIZE);
        size = fread(file_buffer, 1, BUFFER_SIZE-1, file);
        stringstream content;
        content << file_buffer;
        if (send(sock, content.str().c_str(), content.str().size(), 0) < 0) {
            fprintf(stderr, "Write file failed.\n");
            return;
        }
        fsize -= size;

        if (fsize > 0) {
            if (read(sock, buffer, BUFFER_SIZE) < 0) {
                fprintf(stderr, "Read ok failed.\n");
                return;
            }
            if (strcmp(buffer, OK) != 0) {
                fprintf(stderr, "Client not ok.\n");
                return;
            }
        }
    }

    fprintf(stdout, "End send.\n");
    fclose(file);

}

void client_push(int sock, node *curr) {
    char buffer[BUFFER_SIZE];
    fprintf(stdout, "Client push files.\n");
//    struct timeval tmOut;
//    tmOut.tv_sec = 0;
//    tmOut.tv_usec = 0;
//    fd_set fds;
//    FD_ZERO(&fds);
//    FD_SET(sock, &fds);
//    int nRet;
//    char tmp[2];

    if (write(sock, OK, strlen(OK) + 1) < 0) {
        fprintf(stderr, "Write ok failed.\n");
        return;
    }

    if (read(sock, buffer, BUFFER_SIZE) < 0) {
        fprintf(stderr, "Read file name failed.\n");
        return;
    }

    stringstream file_name;
    file_name << buffer;
    fprintf(stdout, "Get file name: %s\n", file_name.str().c_str());

    for (auto &item : fprotected) {
        if (item == file_name.str()) {
            fprintf(stderr, "Conflicts with protected file: %s.\n", file_name.str().c_str());
            if (write(sock, ERR_NULL, strlen(ERR_NULL) + 1) < 0) {
                fprintf(stderr, "Write NULL failed.\n");
            }
            return;
        }
    }

    if (write(sock, OK, strlen(OK) + 1) < 0) {
        fprintf(stderr, "Write ok failed.\n");
        return;
    }

    if (read(sock, buffer, BUFFER_SIZE) < 0) {
        fprintf(stderr, "Read file size failed.\n");
        return;
    }

    long fsize;
    sscanf(buffer, "%ld", &fsize);
    fprintf(stdout, "File size: %ld\n", fsize);

    string path = curr->path + file_name.str();
    FILE *file = fopen(path.c_str(), "w");
    if (!file) {
        fprintf(stderr, "Cannot open file: %s\n", path.c_str());
        if (write(sock, ERR_NULL, strlen(ERR_NULL) + 1) < 0) {
            fprintf(stderr, "Write NULL failed.\n");
        }
        return;
    }

    if (write(sock, OK, strlen(OK) + 1) < 0) {
        fprintf(stderr, "Write ok failed.\n");
        return;
    }

    fprintf(stdout, "Ready to get file.\n");
    long size;
    while (fsize > 0) {
        bzero(buffer, BUFFER_SIZE);
        fprintf(stdout, "%ld left\n", fsize);
//        nRet = select(FD_SETSIZE, &fds, NULL, NULL, &tmOut);
//        if (nRet == 0) {
//            fprintf(stdout, "Buffer empty.\n");
//            fclose(file);
//            return;
//        }
        size = recv(sock, buffer, BUFFER_SIZE, 0);
        if (size <= 0) {
            fprintf(stderr, "Read failed.\n");
            fclose(file);
            return;
        } else {
            fprintf(stdout, "Write file size: %ld\n", size);
        }
        fwrite(buffer, 1, size, file);
        fsize -= size;

        if (fsize > 0) {
            if (write(sock, OK, strlen(OK) + 1) < 0) {
                fprintf(stderr, "Write ok failed.\n");
                return;
            }
        }
    }

    bool isRep = false;
    for(auto &item : curr->child){
        if(item.name == file_name.str()){
            isRep = true;
            break;
        }
    }
    if(!isRep) curr->child.push_back(node(file_name.str(), *curr));
    fclose(file);
    fprintf(stdout, "End push.\n");
}