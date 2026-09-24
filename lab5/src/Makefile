CC=gcc
CFLAGS=-pthread

all: mutex factorial deadlock

mutex: mutex.c
	$(CC) -o mutex mutex.c $(CFLAGS)

factorial: factorial.c
	$(CC) -o factorial factorial.c $(CFLAGS)

deadlock: deadlock.c
	$(CC) -o deadlock deadlock.c $(CFLAGS)

clean:
	rm -f mutex factorial deadlock
