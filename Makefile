CC = gcc
CFLAGS = -Wall -Wextra -std=gnu11
LDFLAGS =
SRC = main.c webserver.c
OBJ = $(SRC:.c=.o)
EXEC = webserver

# Default target
all: $(EXEC)

# Linking the executable
$(EXEC): $(OBJ)
	$(CC) $(OBJ) -o $(EXEC) $(LDFLAGS)

# Compiling .c files to .o files
%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

# Clean target to remove compiled files
clean:
	rm -f $(OBJ) $(EXEC)

run: all
	./webserver

# PHONY target to prevent conflicts with files named 'all' or 'clean'
.PHONY: all clean
