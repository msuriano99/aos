# Compiler and flags
CC = gcc
CFLAGS = -std=c11 -Wall -Wextra -O2 -D_GNU_SOURCE

# Source files
SRC = wish.c

# Output binary
TARGET = wish

# Default target
all: $(TARGET)

# Build the target
$(TARGET): $(SRC)
	$(CC) $(CFLAGS) -o $(TARGET) $(SRC)

# Clean up generated files
clean:
	rm -f $(TARGET)

# Run the program
run: $(TARGET)
	./$(TARGET)
