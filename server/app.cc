#include "server.h"

#define COMMAND_BUFFER 1024
#define QUIT "quit"

extern bool run;


void handle_server_cmd() {
	char server_cmd[COMMAND_BUFFER];
	fprintf(stdout, "Server cmd\n");
	while (run) {
		scanf("%s", server_cmd);
		if (strcmp(server_cmd, QUIT) == 0) {
			fprintf(stdout, "Quit the application.\n");
			run = false;
		}
	}
}


int main(int argc, char * argv[]) {
	int port;
	if (argc != 2) {
		cerr << "You need to input the port number" << endl;
		return 1;
	}
	sscanf(argv[1], "%d", &port);
	printf("port: %d\n", port);
	pthread_t thread;
	pthread_create(&thread, NULL, server, (void*)(&port));
	handle_server_cmd();
	return 0;
}