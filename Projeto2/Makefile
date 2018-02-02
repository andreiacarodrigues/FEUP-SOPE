CC=gcc

CFLAGS=-Wall

all: bin/gerador bin/parque

bin/gerador: gerador.c
	$(CC) $(CFLAGS) gerador.c -o bin/gerador -Wall -pthread

bin/parque: parque.c
	$(CC) $(CFLAGS) parque.c -o bin/parque -Wall -pthread

