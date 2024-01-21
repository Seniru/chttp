CC = gcc
CFLAGS = -Wall -Wextra -g

SRC = main.c url.c http.c
OBJ = $(SRC:.c=.o)

all: prog

prog: $(OBJ)
	$(CC) $(CFLAGS) $^ -o $@

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f prog $(OBJ)
