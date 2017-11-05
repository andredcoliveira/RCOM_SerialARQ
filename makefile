CC=gcc
CFLAGS=-Wall -I.
CFLAGS+=-g
CFLAGS+=-std=gnu99
LDLIBS=-lm

transfer: src/application.c src/datalink.o src/tools.o
	$(CC) src/application.c src/datalink.o src/tools.o -o transfer $(CFLAGS) $(LDLIBS)

clean:
	rm -f transfer src/*.o output.*
