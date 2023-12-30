CC = gcc
CFLAGS = -Wall -Wextra -g

SRC_DIR = src
BIN_DIR = bin

all: $(BIN_DIR)/tarsau

$(BIN_DIR)/tarsau: $(SRC_DIR)/tarsau.c
	$(CC) $(CFLAGS) -o $@ $^

clean:
	rm -f $(BIN_DIR)/tarsau
