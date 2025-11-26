CC = gcc
TARGET = question7
SOURCE = question7.c

# ESSENTIAL FLAGS: -pthread for pthreads, -lrt for clock_gettime
LDFLAGS = -pthread -lrt -lm

$(TARGET): $(SOURCE)
	$(CC) $(SOURCE) -o $(TARGET) $(LDFLAGS)

.PHONY: clean
clean:
	rm -f $(TARGET)