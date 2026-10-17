CC = gcc
CFLAGS = -Wall -Wextra -O3 -pthread
TARGET = parallel_search
SRCS = main.c queue.c search.c
OBJS = $(SRCS:.c=.o)

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJS) $(TARGET) $(TARGET).exe

.PHONY: all clean
