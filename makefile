CC=gcc
CFLAGS = -g
# LIBS = -lsocket -lnsl

all: socket
	
socket: socket.o
	$(CC) -o socket socket.o $(LIBS)

socket.o: socket.c port.h

clean:
	rm -f socket socket.o
