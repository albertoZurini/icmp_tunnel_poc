CC = gcc
CFLAGS = -Wall -Wextra

OBJS = icmp_utils.o
BINS = receive send

all: $(BINS)

icmp_utils.o: icmp_utils.c icmp_utils.h
	$(CC) $(CFLAGS) -c -o $@ icmp_utils.c

receive: receive.c $(OBJS)
	$(CC) $(CFLAGS) -o $@ receive.c $(OBJS)

send: send.c $(OBJS)
	$(CC) $(CFLAGS) -o $@ send.c $(OBJS)

clean:
	rm -f $(OBJS) $(BINS)

.PHONY: all clean
