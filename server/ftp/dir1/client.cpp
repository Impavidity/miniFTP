#include "client.h"

bool DEBUG = false;

void client(int port){
    bool run = true;

    char buffer[BUFFER_SIZE];
    string command;
    vector<string> cmds;

    int sock_msg;
    struct hostent* host;
    struct sockaddr_in server_msg;

    host = gethostbyname(server_name);
    if(!host){
        fprintf(stderr, "Get host by name failed.");
        return;
    }

    sock_msg = socket(AF_INET, SOCK_STREAM, 0);
    if(sock_msg < 0){
        fprintf(stderr, "Open stream socket failed.");
        return;
    }


    server_msg.sin_family = AF_INET;
    server_msg.sin_port = htons(port);
    memcpy((char*)&server_msg.sin_addr, host->h_addr_list[0], (size_t)host->h_length);

    if(connect(sock_msg, (struct sockaddr*)&server_msg, sizeof(server_msg)) < 0){
        fprintf(stderr, "Connect stream socket failed.\n");
        return;
    }

    fprintf(stdout, "%s\n", "Client in.\n");
    if (read(sock_msg, buffer, sizeof(buffer)) < 0){
        fprintf(stderr, "Read failed.\n");
    }
    fprintf(stdout, "%s\n", buffer);

    while(true){
        if(not run){
            fprintf(stdout, "End the client.\n");
            return;
        }

        getline(cin, command);
        boost::split(cmds, command,boost::is_any_of(" \t"));
        if(cmds.size() < 1) continue;
        const char* cmd = cmds[0].c_str();

        if(cmds.size()==1 && strcmp(cmd, CMD_QUIT) == 0){
            run = false;
        }else if(strcmp(cmd, CMD_LS) == 0){
            cmd_list(sock_msg);
        }else if(cmds.size()==1 && strcmp(cmd, CMD_HELP) == 0){
            cmd_help(sock_msg);
        }else if(cmds.size()==1 && strcmp(cmd, CMD_DEBUG) == 0){
            DEBUG = true;
            fprintf(stdout, "Debug\n");
        }else if(strcmp(cmd, CMD_NO) == 0){
            if(cmds.size() == 2){
                if(cmds[1] == CMD_DEBUG){
                    DEBUG = false;
                    fprintf(stdout, "No debug\n");
                }
                else{
                    fprintf(stderr, "Command not found.\n");
                }
            }
            else{
                fprintf(stderr, "Command not found.\n");
            }
        }else if(strcmp(cmd, CMD_PULL) == 0){
            cmd_pull(sock_msg, cmds);
        }else if(strcmp(cmd, CMD_PUSH) == 0){
            cmd_push(sock_msg, cmds);
        } else if(strcmp(cmd, CMD_CD) == 0){
            cmd_cd(sock_msg, cmds);
        } else if(strcmp(cmd, CMD_PWD) == 0){
            cmd_pwd(sock_msg);
        }else{
            fprintf(stderr, "Command not found.\n");
        }
    }
}

void cmd_pwd(int sock){
    char buffer[BUFFER_SIZE];

    if(write(sock, CMD_PWD, strlen(CMD_PWD)+1) < 0){
        fprintf(stderr, "Write failed.\n");
        return;
    }
    if(read(sock, buffer, sizeof(buffer)) < 0){
        fprintf(stderr, "Read failed.\n");
        return;
    }
    fprintf(stdout, "Currenct directory: %s\n", buffer);
}
void cmd_list(int sock){
    char buffer[BUFFER_SIZE];

    if(DEBUG)
        fprintf(stdout, "list files\n");
    write(sock, CMD_LS, sizeof(CMD_LS) + 1);
    if (read(sock, buffer, sizeof(buffer)) < 0){
        fprintf(stderr, "Read failed.\n");
        return;
    }
    fprintf(stdout, "%s\n", buffer);
}

void cmd_help(int sock){
    char buffer[BUFFER_SIZE];

    if(DEBUG)
        fprintf(stdout, "get help\n");
    write(sock, CMD_HELP, sizeof(CMD_HELP) + 1);
    if (read(sock, buffer, sizeof(buffer)) < 0){
        fprintf(stderr, "Read failed.\n");
        return;
    }
    fprintf(stdout, "%s\n", buffer);
}

void cmd_cd(int sock, vector<string>& cmd){
    char buffer[BUFFER_SIZE];

    if(cmd.size()>2){
        fprintf(stderr, "There    char buffer[BUFFER_SIZE]; are too many arguments.\n");
        return;
    }
    auto dir = cmd[1];
    if(write(sock, CMD_CD, strlen(CMD_CD)+1) < 0){
        fprintf(stderr, "Write failed.\n");
        return;
    }
    fprintf(stdout, "%s\n", cmd[1].c_str());

    if(read(sock, buffer, BUFFER_SIZE) < 0){
        fprintf(stderr, "Read ok failed.\n");
        return;
    }
    if(strcmp(buffer, OK) != 0){
        fprintf(stderr, "Server not ok.\n");
        return;
    }

    if(write(sock, dir.c_str(), dir.size()+1) < 0){
        fprintf(stderr, "Write failed.\n");
        return;
    }

    if(read(sock, buffer, BUFFER_SIZE) < 0){
        fprintf(stderr, "Read failed.\n");
        return;
    }

    fprintf(stdout, "Current directory: %s\n", buffer);
}

void cmd_pull(int sock, vector<string>& cmd){
    char buffer[BUFFER_SIZE];
    struct timeval tmOut;
    tmOut.tv_sec = 0;
    tmOut.tv_usec = 0;
    fd_set fds;
    FD_ZERO(&fds);
    FD_SET(sock, &fds);
    int nRet; char tmp[2];


    if(cmd.size()>2){
        fprintf(stderr, "There are too many arguments.\n");
        return;
    }
    if(cmd.size()==1){
        fprintf(stderr, "There isn't any file name.\n");
        return;
    }

    if(send(sock, CMD_PULL, strlen(CMD_PULL) + 1, 0) < 0){
        fprintf(stderr, "Write pull failed.\n");
        return;
    }

    if(read(sock, buffer, BUFFER_SIZE) < 0 ){
        fprintf(stderr, "Read ok failed.\n");
        return;
    }
    if(strcmp(buffer, OK)!=0){
        fprintf(stderr, "Server not ok.\n");
        return;
    }

    if(send(sock, cmd[1].c_str(), cmd[1].size() + 1, 0) < 0){
        fprintf(stderr, "Write file name failed.\n");
        return;
    }

    if(read(sock, buffer, BUFFER_SIZE) < 0){
        fprintf(stderr, "Read failed.\n");
        return;
    }

    if(strcmp(buffer, ERR_NULL) == 0){
        fprintf(stderr, "Pull null.\n");
        return;
    }

    if(write(sock, OK, strlen(OK)+1) < 0){
        fprintf(stderr, "Write ok failed.\n");
        return;
    }

    long fsize;
    sscanf(buffer, "%ld", &fsize);
    fprintf(stdout, "File size: %ld\n", fsize);

    FILE* file = fopen(cmd[1].c_str(), "w");
    long size=0;
    char file_buffer[BUFFER_SIZE];
    while(fsize>0){
        bzero(file_buffer, BUFFER_SIZE);
        fprintf(stdout, "%ld left\n", fsize);
        nRet = select(FD_SETSIZE, &fds, NULL, NULL, &tmOut);
        if(nRet==0){
            fprintf(stdout, "Buffer empty.\n");
            fclose(file);
            return;
        }
        size = recv(sock, file_buffer, BUFFER_SIZE, 0);
        if(size<=0){
            fprintf(stderr, "Read failed.\n");
            fclose(file);
            return;
        }
        fwrite(file_buffer, 1, size, file);
        fsize-=size;

        if(fsize>0){
            if(write(sock, OK, strlen(OK)+1) < 0){
                fprintf(stderr, "Write ok failed.\n");
                return;
            }
        }
    }

    fclose(file);

    fprintf(stdout, "End pull.\n");
}

void cmd_push(int sock, vector<string>& cmd){
    char buffer[BUFFER_SIZE];

    if(write(sock, CMD_PUSH, strlen(CMD_PUSH) + 1) < 0){
        fprintf(stderr, "Write push failed.\n");
        return;
    }

    if(cmd.size() > 2){
        fprintf(stderr, "There are too many arguments.\n");
        return;
    }

    auto file_name = cmd[1];

    if(read(sock, buffer, BUFFER_SIZE) < 0){
        fprintf(stderr, "Read failed.\n");
        return;
    }

    if(strcmp(buffer, OK) != 0){
        fprintf(stderr, "Server not ok.\n");
        return;
    }

    if(write(sock, file_name.c_str(), file_name.size()+1) < 0){
        fprintf(stderr, "Write file name failed.\n");
        return;
    }

    if(read(sock, buffer, BUFFER_SIZE) < 0){
        fprintf(stderr, "Read failed.\n");
        return;
    }

    if(strcmp(buffer, ERR_NULL) == 0){
        fprintf(stderr, "Push null: server refuse the push.\n");
        return;
    }

    FILE* file = fopen(file_name.c_str(), "r");
    if(!file){
        fprintf(stderr, "No such file: %s\n", file_name.c_str());
        return;
    }

    fseek(file, 0, SEEK_END);
    long fsize = ftell(file);
    stringstream fstrsize;
    fstrsize << fsize;
    fseek(file, 0, SEEK_SET);

    if(write(sock, fstrsize.str().c_str(), fstrsize.str().size()+1) < 0){
        fprintf(stderr, "Write file size failed.\n");
        return;
    }

    if(read(sock, buffer, BUFFER_SIZE) < 0){
        fprintf(stderr, "Read failed.\n");
        return;
    }

    if(strcmp(buffer, OK) != 0){
        fprintf(stderr, "Server not ok.\n");
        return;
    }else{
        fprintf(stdout, "Server is ok.\n");
    }


    long size;
    while(fsize > 0){
        bzero(buffer, BUFFER_SIZE);
        size = fread(buffer, 1, BUFFER_SIZE-1, file);
        fprintf(stderr,"%s\n", buffer);
        stringstream content;
        content<<buffer;
        if(send(sock, content.str().c_str(), content.str().size(), 0) < 0){
            fprintf(stderr, "Write file failed.\n");
            return;
        }
        fsize -= size;

        if(fsize > 0){
            if(read(sock, buffer, BUFFER_SIZE) < 0){
                fprintf(stderr, "Read failed.\n");
                return;
            }
            if(strcmp(buffer, OK)!=0){
                fprintf(stderr, "Server not ok.\n");
                return;
            }
        }
    }

    fclose(file);
    fprintf(stdout, "End push.\n");
}