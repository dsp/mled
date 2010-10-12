CFLAGS=-O2 -Wall

mled: src/mled.c
	gcc $(CFLAGS) -o mled src/mled.c

