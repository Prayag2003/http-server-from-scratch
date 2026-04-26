CC = gcc
CFLAGS = -Wall -Wextra -std=c17 -g -Werror -pthread -O3 -flto
LDFLAGS = -lm

TARGET = server
SRC = server.c utils/http_response.c utils/http_serve.c
OBJ = $(SRC:.c=.o)

all: $(TARGET)

$(TARGET): $(OBJ)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

run: $(TARGET)
	./$(TARGET)

clean:
	rm -f $(OBJ) $(TARGET)

.PHONY: all run clean
