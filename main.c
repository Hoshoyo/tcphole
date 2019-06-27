#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/ip.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <pthread.h>

void* listen_thread(void *ptr) {
    printf("%s\n", (char*)ptr);
}

void* connect_thread(void* ptr) {
    printf("%s\n", (char*)ptr);
}

/* Structure describing an Internet socket address.  */
int main() {
    pthread_t thread1, thread2;

    int iret1 = pthread_create( &thread1, NULL, listen_thread, (void*)"thread 1");
    if(iret1)
    {
        fprintf(stderr,"Error - pthread_create() return code: %d\n", iret1);
        return -1;
    }

    int iret2 = pthread_create( &thread2, NULL, connect_thread, (void*)"thread 2");
    if(iret2)
    {
        fprintf(stderr,"Error - pthread_create() return code: %d\n", iret1);
        return -1;
    }

    pthread_join(thread1, NULL);
    pthread_join(thread2, NULL);

#if 0
	int socket_desc;
	int client_sock;
	int c = 16;
	int read_size = 0;

	struct sockaddr_in server;
	struct sockaddr_in client;

	char client_message[2048];

	//Create socket
    socket_desc = socket(AF_INET, SOCK_STREAM , 0);
    int enable = 1;
    if (setsockopt(socket_desc, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(int)) < 0) {
        printf("setsockopt(SO_REUSEADDR) failed");
        return -1;
    }

    if (socket_desc == -1) {
        printf("Could not create socket");
		return -1;
    } else {
    	printf("Socket created\n");
	}

	//Prepare the sockaddr_in structure
    server.sin_family = AF_INET;
    server.sin_addr.s_addr = INADDR_ANY;
    server.sin_port = htons(8888);

	if (bind(socket_desc, (const struct sockaddr *)&server, 16) < 0) {
		printf("bind failed\n");
		return -1;
	}

	listen(socket_desc, 3);
	printf("Waiting for connections...\n");

	client_sock = accept(socket_desc, (struct sockaddr *)&client, (unsigned int *)&c);

	if (client_sock < 0) {
		printf("accept failed\n");
		return -1;
	}

	printf("Connection accepted!\n");

	read_size = recv(client_sock, (void*)client_message , 2000 , 0);
	while (read_size > 0) {
        //Send the message back to client
        write(client_sock, (void*)client_message, strlen(client_message));
		read_size = recv(client_sock, (void*)client_message , 2000 , 0);
    }

	if (read_size == 0) {
		printf("Client disconnected\n");
	} else if (read_size == -1) {
		printf("recv failed");
	}
#endif
	return 0;
}