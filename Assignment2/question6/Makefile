CC = gcc
CFLAGS = -pthread -O0

all: pthread_sum

pthread_sum: pthread_sum.c
	$(CC) $(CFLAGS) -o pthread_sum pthread_sum.c

clean:
	rm -f pthread_sum
