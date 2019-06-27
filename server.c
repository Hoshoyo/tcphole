#include "common.h"
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/ip.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <errno.h>
#include <pthread.h>

#define LISTEN_PORT 7777

typedef struct {
	int socket;
	struct sockaddr_in client_info;	
} Connection;

static Connection clients[2];
static pthread_t clients_thread[2];

void* handle_client(void* info) {
	int index = (int)(unsigned long int)info;

	char buffer[1024] = {0};
	char client_message[1024] = {0};
	int read_size = recv(clients[index].socket, (void*)client_message, 2048, 0);
	if (read_size == 0) {
		printf("Client disconnected\n");
	} else if (read_size == -1) {
		printf("recv failed");
	} else {
		printf("%s\n", client_message);
	}

	{
        //Send the message back to client
		ClientInfo info = {0};
		info.index = index;
		info.type = INFO_SELF_PUBLIC;
		info.local_ip = clients[index].client_info.sin_addr;
		info.local_port = clients[index].client_info.sin_port;
		memcpy(buffer, &info, sizeof(info));

        if(send(clients[index].socket, buffer, sizeof(info), 0) > 0) {
			printf("Sent to client\n");
		} else {
			printf("Error sending data: %s\n", strerror(errno));
		}
    }

}

int main(int argc, char** argv) {
	int socket_desc;
	int c = 16;

	struct sockaddr_in server;

	//Create socket
    socket_desc = socket(AF_INET, SOCK_STREAM , 0);

    if (socket_desc == -1) {
        printf("Could not create socket: %s\n", strerror(errno));
		return -1;
    }
	
	int enable = 1;
    if (setsockopt(socket_desc, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(int)) < 0) {
        printf("setsockopt(SO_REUSEADDR) failed: %s\n", strerror(errno));
        return 0;
    }

	printf("Socket created\n");

	//Prepare the sockaddr_in structure
    server.sin_family = AF_INET;
    server.sin_addr.s_addr = INADDR_ANY;
    server.sin_port = htons(LISTEN_PORT);

	if (bind(socket_desc, (const struct sockaddr *)&server, 16) < 0) {
		printf("Bind failed: %s\n", strerror(errno));
		return -1;
	}

	listen(socket_desc, 3);
	printf("Waiting for connections...\n");

	while(1) {
		for(int index = 0; index < 2; ++index) {
			clients[index].socket = accept(socket_desc, (struct sockaddr *)&clients[index].client_info, (unsigned int *)&c);

			if (clients[index].socket < 0) {
				printf("Accept failed: %s\n", strerror(errno));
				return -1;
			}

			printf("Connection accepted %d!\n", index);

			int iret = pthread_create(&clients_thread[index], NULL, handle_client, (void*)(unsigned long int)index);
			if(iret)
			{
				fprintf(stderr,"Error - pthread_create() return code: %d\n", iret);
				return -1;
			}
		}
		pthread_join(clients_thread[0], 0);
		pthread_join(clients_thread[1], 0);

		// Both clients are here, send each other's info
		{
			// Client 0 receives client 1 info
			ClientInfo info = {0};
			info.type = INFO_REMOTE_PUBLIC;
			info.index = 0;
			info.remote_ip = clients[1].client_info.sin_addr;
			info.remote_port = clients[1].client_info.sin_port;

			if(send(clients[0].socket, &info, sizeof(info), 0) > 0) {
				printf("Sent to client 0, client's 1 info\n");
			} else {
				printf("Error sending data to client 0: %s\n", strerror(errno));
			}
			close(clients[0].socket);
		}
		{
			// Client 1 receives client 0 info
			ClientInfo info = {0};
			info.type = INFO_REMOTE_PUBLIC;
			info.index = 1;
			info.remote_ip = clients[0].client_info.sin_addr;
			info.remote_port = clients[0].client_info.sin_port;

			if(send(clients[1].socket, &info, sizeof(info), 0) > 0) {
				printf("Sent to client 1, client's 0 info\n");
			} else {
				printf("Error sending data to client 0: %s\n", strerror(errno));
			}
			close(clients[1].socket);
		}
	}

    return 0;
}