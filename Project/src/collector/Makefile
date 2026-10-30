CC = gcc
CFLAGS = -Wall -Wextra -O2 -Iinclude -D_GNU_SOURCE
LDFLAGS = 

TARGET = ../kernel_collector
SRCS = src/main.c src/proc_parser.c src/syscalls.c src/kthreads.c
OBJS = $(SRCS:.c=.o)

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f src/*.o $(TARGET)

.PHONY: all clean
