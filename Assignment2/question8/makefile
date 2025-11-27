CC = gcc
TARGET = question8
SOURCE = question8.c

# ESSENTIAL FLAGS: -pthread for pthreads
LDFLAGS = -pthread 

$(TARGET): $(SOURCE)
	$(CC) $(SOURCE) -o $(TARGET) $(LDFLAGS)

.PHONY: clean
clean:
	rm -f $(TARGET)