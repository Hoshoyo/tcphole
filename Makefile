all:
	gcc -g server.c -o server -pthread
	gcc -g client.c -o client -pthread