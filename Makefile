CC = gcc
CFLAGS = -std=gnu99 -pedantic -Wall -Wextra
LIBS = -lrt -lm

SOURCES = $(wildcard *.c)
OBJS = $(SOURCES:.c=.o)

color: $(OBJS)
	$(CC) $(CFLAGS) -o color $^ $(LIBS)

.c.o:
	$(CC) $(CFLAGS) -c -o $@ $< $(LIBS)
