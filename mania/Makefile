CC = gcc
CFLAGS = -Wall -Wextra -std=c11 $(shell pkg-config --cflags libvlc)
LDFLAGS = -lncurses $(shell pkg-config --libs libvlc)

OBJS = main.o level.o

all: mania

mania: $(OBJS)
	$(CC) $(OBJS) -o mania $(LDFLAGS)

%.o: %.c level.h
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f mania $(OBJS)

.PHONY: all clean
