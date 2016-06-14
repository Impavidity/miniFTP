#include "server.h"

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


bool run = true;

void *server(void* listen_port) {
	int port = *((int*)(listen_port));
	int sock_msg;
	struct sockaddr_in server_msg;

	sock_msg = socket(AF_INET, SOCK_STREAM, 0);
	if (sock_msg < 0) {
		cerr << "Open stream socket failed." << endl;
		ERR = STREAM_SOCKET_ERROR;
		return NULL;
	}

	// set the attributes of the server
	server_msg.sin_family = AF_INET;
	server_msg.sin_addr.s_addr = INADDR_ANY;
	server_msg.sin_port = htons(port);

	// bind the port for each socket
	if (bind(sock_msg, (struct sockaddr *) & server_msg, sizeof(server_msg)) < 0) {
		cerr << "Bind stream socket failed." << endl;
		ERR = BIND_SOCKET_ERROR;
		return NULL;
	}

	cerr << "Socket port: " << ntohs(server_msg.sin_port) << endl;
	listen(sock_msg, 2);

	while (run) {
		int msg_sock;
		int msg_len;
		msg_len = sizeof(struct sockaddr_in);
		msg_sock = accept(sock_msg, (struct sockaddr *)0, (socklen_t *) &msg_len);

		if (msg_sock == -1) {
			cerr << "Accept Failed" << endl;
			ERR = ACCEPT_ERROR;
		} else {
			pthread_t thread;
			pthread_create(&thread, NULL, new_service,(void *)&msg_sock);
		}
	}
}

void *new_service(void * sock) {
	int msg_sock = *((int *)sock);
	char buffer[BUFFER_SIZE];
	cerr << "I am in new Service" << endl;

	node root = node(ROOT);
	traverse_dir(root, ROOT);
	node * curr = &root;

	cerr << "Connection success." << endl;

	int snd_size = 0, rcv_size = 0;
	socklen_t optlen = sizeof(snd_size);

	int err;
	err = getsockopt(*(int *) sock, SOL_SOCKET, SO_SNDBUF, &snd_size, &optlen);
	if (err < 0) {
		cerr << "Error get send buffer size." << endl;
	} else {
		cerr << "Send buffer size " << snd_size << endl;
	}
	err = getsockopt(*(int *) sock, SOL_SOCKET, SO_RCVBUF, &rcv_size, &optlen);
	if (err < 0) {
		cerr << "Error get rcv buffer size." << endl;
	} else {
		cerr << "Send rcv size " << rcv_size << endl;
	}

	while (run) {
		auto read_flag = read(msg_sock, buffer, sizeof(buffer));
		if (read_flag < 0) {
			cerr << "Reading from client failed." << endl;
			close(msg_sock);
			break;
		} else if (read_flag == 0) {
			cerr << "Connection ended" << endl;
			close(msg_sock);
		} else {
			if (strcmp(buffer, CMD_HELP) == 0) {
				cmd_help(msg_sock);
			} else if (strcmp(buffer, CMD_LS) == 0) {
				cmd_list(msg_sock, curr);
			} else {
				cerr << "Command not found " << buffer << endl;
			}
		}
	}

}


void cmd_help(int sock) {
	cerr << "Command help" << endl;
	if (write(sock, help.c_str(), help.size() + 1) < 0) {
		cerr << "Write Failed" << endl;
	} else {
		cerr << "Send help message success"  << endl;
	}
}

void cmd_list(int sock, node * cur) {
	cerr << "Command list" << endl;
	auto list = list_dir(cur);
	if (write(sock, list.c_str(), strlen(list.c_str()) + 1) < 0) {
		cerr << "Write failed" << endl;
	} else {
		cerr << "Send ls message success" << endl;
	}
}

void client_pwd(int sock, node *& curr) {
	cerr << "Command pwd" << endl;
	if (write(sock, curr->name.c_str(), curr->name.size()+1) < 0) {
		cerr << "write failed" << endl;
		return;
	} else {
		cerr << "Send pwd message success" << endl;
	}
}

void client_cd(int sock, node *&curr) {
	char buffer[BUFFER_SIZE];
	if (send(sock, OK, strlen(OK) + 1, 0) < 0) {
		cerr << "write ok failed" << endl;
		return;
	}
	cerr << "command cd" << endl;
}