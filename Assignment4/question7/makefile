# Compiler
CC = gcc

# Compiler flags, get some nice warnings
# Added -pthread because you are using the pthread library!
CFLAGS = -Wall -Wextra -pthread

# Find source code
SOURCES = $(wildcard *.c)

# List of executables
EXECUTABLES = $(SOURCES:.c=)

# Build all executables "make"
all: $(EXECUTABLES)

# To compile
%: %.c
	$(CC) $(CFLAGS) -o $@ $<

# Clean executables "make clean"
clean:
	rm -f $(EXECUTABLES)