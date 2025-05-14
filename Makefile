# Compiler and flags
CC = gcc
CFLAGS = -Wall -Wextra -g

# Targets
TARGETS = part1 part2 part3 part4

# Default target
all: $(TARGETS)

# Build rules
part1: Part1.c common.c
	$(CC) $(CFLAGS) -o part1 Part1.c common.c

part2: Part2.c common.c
	$(CC) $(CFLAGS) -o part2 Part2.c common.c

part3: Part3.c common.c
	$(CC) $(CFLAGS) -o part3 Part3.c common.c

part4: Part4.c common.c
	$(CC) $(CFLAGS) -o part4 Part4.c common.c

# Clean rule
clean:
	rm -f $(TARGETS)
