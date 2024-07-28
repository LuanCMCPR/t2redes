CC = gcc
CFLAGS = -Wall -Wextra -Werror
LDFLAGS = -lpthread

SRC = network_test.c network.c packet.c
OBJ = $(SRC:.c=.o)
EXEC = network_test

all: $(EXEC)

$(EXEC): $(OBJ)
	$(CC) $(OBJ) -o $(EXEC) $(LDFLAGS)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJ) $(EXEC)

.PHONY: clean
