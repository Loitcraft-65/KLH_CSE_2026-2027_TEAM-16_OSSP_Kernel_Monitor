CC = gcc
CFLAGS = -std=c11 -O2 -Wall -Wextra -Wpedantic

.PHONY: all clean
all: kernel_collector

kernel_collector: collector.c
	$(CC) $(CFLAGS) collector.c -o kernel_collector

clean:
	rm -f kernel_collector
