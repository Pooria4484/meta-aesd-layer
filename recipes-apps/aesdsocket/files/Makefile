# Makefile
CC ?= gcc
CFLAGS ?= -Wall -Wextra -g
LDFLAGS ?= -pthread

TARGET = aesdsocket

all: $(TARGET)

$(TARGET): aesdsocket.o
	$(CC) $(LDFLAGS) $^ -o $@

aesdsocket.o: aesdsocket.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(TARGET) *.o
	rm -f /var/tmp/aesdsocketdata

.PHONY: all clean
