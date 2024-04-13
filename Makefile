CC=gcc
CFLAGS=-Wall -g
TARGET1=client1
TARGET2=client2

all: $(TARGET1) $(TARGET2)

$(TARGET1): client1.c
	$(CC) $(CFLAGS) -o $(TARGET1) client1.c

$(TARGET2): client2.c
	$(CC) $(CFLAGS) -o $(TARGET2) client2.c

clean:
	rm -f $(TARGET1) $(TARGET2)

.PHONY: all clean
