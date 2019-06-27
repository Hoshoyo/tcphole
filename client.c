#include "common.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/ip.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <errno.h>

#define BIG_ENDIAN_16(X) ((X >> 8) | ((X & 0xff) << 8))

#define PORT 7777
#define SERVER_IP "68.183.105.210"
//#define SERVER_IP "127.0.0.1"

void print_ipv4(unsigned int ip) {
    printf("%d.%d.%d.%d", (ip & 0xff), ((ip & 0xff00) >> 8), ((ip & 0xff0000) >> 16), ((ip & 0xff000000) >> 24));
}
void print_port(unsigned short port) {
    printf("%d", (port >> 8) | ((port & 0xff) << 8));
}

void print_bytes(char* buffer, int length) {
    for(int i = 0; i < length; ++i) {
        printf("%d ", buffer[i]);
    }
}

void* client_listen(void* p) {
    ClientInfo* info = (ClientInfo*)p;

    // Create socket on client's public endpoint
    int listen_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (listen_socket == -1) {
        printf("Could not create socket: %s\n", strerror(errno));
		return 0;
    } else {
    	printf("Listen socket created\n");
	}
    int enable = 1;
    if (setsockopt(listen_socket, SOL_SOCKET, SO_REUSEADDR|SO_REUSEPORT, &enable, sizeof(int)) < 0) {
        printf("setsockopt(SO_REUSEADDR) failed: %s\n", strerror(errno));
        return 0;
    }

    struct sockaddr_in server;

    //Prepare the sockaddr_in structure
    server.sin_family = AF_INET;
    server.sin_addr.s_addr = INADDR_ANY;
    server.sin_port = info->local_port;

    printf("Attempting to bind listening to: %d\n", BIG_ENDIAN_16(info->local_port));

	if (bind(listen_socket, (const struct sockaddr *)&server, 16) < 0) {
		printf("bind failed: %s\n", strerror(errno));
		return 0;
	}

    // Listen
    listen(listen_socket, 1);
    printf("Waiting for connections on ");
    print_port(info->local_port);
    printf("\n");

    struct sockaddr_in peer_info = {0};
    unsigned int c = 16;
    int peer = accept(listen_socket, (struct sockaddr*)&peer_info, &c);

    while(1) {
        if(peer == -1) {
            printf("Error in connection to peer: %s\n", strerror(errno));
        } else {
            printf("Successfully connected to peer!\n");
            print_ipv4(peer_info.sin_addr.s_addr);
            printf(":");
            print_port(peer_info.sin_port);
            printf("\n");
            break;
        }
        sleep(5);
    }
}

int main() { 
    int sockfd, connfd; 
    struct sockaddr_in servaddr = {0};
    struct sockaddr_in cli = {0};

    // Create socket with the randezvous server
    sockfd = socket(AF_INET, SOCK_STREAM, 0); 
    if (sockfd == -1) { 
        printf("socket creation failed...\n"); 
        return -1;
    } else {
        printf("Socket successfully created..\n"); 
    }

    // Enable this socket to reuse address
    int enable = 1;
    if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR|SO_REUSEPORT, &enable, sizeof(int)) < 0) {
        printf("setsockopt(SO_REUSEADDR) failed");
        return -1;
    }

    // Assign server IP, PORT 
    servaddr.sin_family = AF_INET; 
    servaddr.sin_addr.s_addr = inet_addr(SERVER_IP);
    servaddr.sin_port = htons(PORT); 

    // connect the client socket to server socket 
    if (connect(sockfd, (struct sockaddr *)&servaddr, sizeof(servaddr)) != 0) { 
        printf("connection with the server failed...\n"); 
        return -1;
    } else {
        printf("connected to the server..\n"); 
    }

    printf("Sending data to server...\n");
    if(send(sockfd, "Hello", 5, MSG_DONTWAIT) == -1) {
        printf("failed to send data to server: %s\n", strerror(errno));
        close(sockfd);
        return -1;
    }

    printf("Waiting for server data...\n");

    char buffer[2048] = {0};
    int bytes = recv(sockfd, buffer, 2048, 0);
    if(bytes == -1) {
        printf("Failed to receive data from server: %s\n", strerror(errno));
        return -1;
    } else if(bytes == 0) {
        printf("Server disconnected\n");
        return -1;
    }

    // Received data
    ClientInfo public_info = {0};
    memcpy(&public_info, buffer, sizeof(ClientInfo));

    printf("Server: ");
    print_ipv4(public_info.local_ip.s_addr);
    printf(":");
    print_port(public_info.local_port);
    printf("\n");

    // Try to listen on that port
    pthread_t listen_thread;
    int iret = pthread_create(&listen_thread, NULL, client_listen, (void*)&public_info);
    if(iret)
    {
        fprintf(stderr,"Error - pthread_create() return code: %d\n", iret);
        return -1;
    }

    ClientInfo remote_info = {0};
    // Wait for the information on the other client
    int bytes_received = recv(sockfd, &remote_info, sizeof(remote_info), 0);
    if(bytes == -1) {
        printf("Failed to receive data from server: %s\n", strerror(errno));
        return -1;
    } else if(bytes == 0) {
        printf("Server disconnected\n");
        return -1;
    }
    // Successfully received the information from the server! Try to connect directly
    printf("Remote: ");
    print_ipv4(remote_info.remote_ip.s_addr);
    printf(":");
    print_port(remote_info.remote_port);
    printf("\n");
    //close(sockfd);

    int peer_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (setsockopt(peer_socket, SOL_SOCKET, SO_REUSEADDR|SO_REUSEPORT, &enable, sizeof(int)) < 0) {
        printf("setsockopt(SO_REUSEADDR) failed");
        return -1;
    }

    struct sockaddr_in dummy = {0};
    dummy.sin_family = AF_INET;
    dummy.sin_addr.s_addr = INADDR_ANY;
    dummy.sin_port = public_info.local_port;

    if (bind(peer_socket, (const struct sockaddr *)&dummy, sizeof(dummy)) < 0) {
		printf("Trying to bind to the same port failed: %s\n", strerror(errno));
	}

    struct sockaddr_in peer_addr = {0};
    peer_addr.sin_family = AF_INET;
    peer_addr.sin_addr.s_addr = remote_info.remote_ip.s_addr;
    peer_addr.sin_port = remote_info.remote_port;

    while(1) {
        if(connect(peer_socket, (struct sockaddr *)&peer_addr, sizeof(servaddr)) != 0) { 
            printf("connection with the peer failed: %s\n", strerror(errno));
        } else {
            printf("Connected to peer!\n");
            break;
        }
        sleep(5);
    }

    // Wait listening thread
    pthread_join(listen_thread, 0);

    return 0;
}