CC=gcc
CFLAGS=-Wall -g
TARGET=client1

all: $(TARGET)

$(TARGET): client1.c
	$(CC) $(CFLAGS) -o $(TARGET) client1.c

clean:
	rm -f $(TARGET)

.PHONY: clean
