CC=gcc
CFLAGS=-I. -pthread

all: server client

server: server.c libmultmodulo.a
	$(CC) -o server server.c libmultmodulo.a $(CFLAGS)

client: client.c libmultmodulo.a
	$(CC) -o client client.c libmultmodulo.a $(CFLAGS)

libmultmodulo.a: multmodulo.o
	ar rcs libmultmodulo.a multmodulo.o

multmodulo.o: multmodulo.c multmodulo.h
	$(CC) -o multmodulo.o -c multmodulo.c $(CFLAGS)

clean:
	rm -f multmodulo.o libmultmodulo.a server client
