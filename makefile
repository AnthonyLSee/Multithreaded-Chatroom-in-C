CC = gcc
CFLAGS = -w -o
#CFLAGS = -o 

all: client.c server.c
	$(CC) $(CFLAGS) client client.c -lpthread
	$(CC) $(CFLAGS) server server.c -lpthread
	
runC: client
	./client 127.0.0.1

runS: server
	./server
	
clean:
	rm -i client server


