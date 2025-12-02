# Compiler
CC = gcc

# Compiler flags, get some nice warnings
CFLAGS = -Wall -Wextra

# Linker flags
LDFLAGS = -lpthread -lrt

# Find source code
SOURCES = $(wildcard *.c)

# List of executables
EXECUTABLES = $(SOURCES:.c=)

# Build all executables "make"
all: $(EXECUTABLES)

# To compile
%: %.c
	$(CC) $(CFLAGS) -o $@ $< $(LDFLAGS)

# Clean executables "make clean"
clean:
	rm -f $(EXECUTABLES)