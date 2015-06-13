CC = gcc
CFLAGS = -std=gnu99 -pedantic -Wall -Wextra
LIBS = -lrt -lm -lmicrohttpd
BINARY = color

SOURCES = $(wildcard *.c)
OBJS = $(SOURCES:.c=.o)


$(BINARY): $(OBJS)
	$(CC) $(CFLAGS) -o color $^ $(LIBS)

.c.o:
	$(CC) $(CFLAGS) -c -o $@ $< $(LIBS)

clean:
	rm -f $(OBJS) $(BINARY)

.PHONY: clean
