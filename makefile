all:
	make server
	make client
clean:
	rm server
	rm client

server: server.c
	gcc -Wall -ansi -o server server.c -lpthread

client: client.c
	gcc -Wall -ansi -o client client.c -lpthread


