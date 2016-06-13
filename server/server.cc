#include "server.h"

SERVER_ERROR ERR;

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

}