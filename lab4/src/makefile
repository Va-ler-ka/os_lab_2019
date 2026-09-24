CC=gcc
CFLAGS=-I.

all: parallel_min_max process_memory zombie

parallel_min_max: parallel_min_max.c utils.o find_min_max.o
	$(CC) -o parallel_min_max parallel_min_max.c utils.o find_min_max.o $(CFLAGS)

process_memory: process_memory.c
	$(CC) -o process_memory process_memory.c $(CFLAGS)

zombie: zombie.c
	$(CC) -o zombie zombie.c $(CFLAGS)

utils.o: utils.c utils.h
	$(CC) -o utils.o -c utils.c $(CFLAGS)

find_min_max.o: find_min_max.c find_min_max.h utils.h
	$(CC) -o find_min_max.o -c find_min_max.c $(CFLAGS)

clean:
	rm -f utils.o find_min_max.o parallel_min_max process_memory zombie min_max_*.txt
