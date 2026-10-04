CC = gcc
CFLAGS = -Wall -Wextra -Wpedantic -std=c11 -O2 -g
CPPFLAGS = -Iinclude
LIBS = -lutil

SRC = \
	src/main.c \
	src/pty.c \
	src/session.c \
	src/terminal.c \
	src/multiplexer.c \
	src/commands.c \
	src/signal.c \
	src/buffer.c \
	src/server.c \
	src/client.c \
	src/socket.c

OBJ = $(SRC:.c=.o)

TARGET = ptymux

all: $(TARGET)

$(TARGET): $(OBJ)
	$(CC) $(OBJ) -o $(TARGET) $(LIBS)

%.o: %.c
	$(CC) $(CFLAGS) $(CPPFLAGS) -c $< -o $@

clean:
	rm -f $(OBJ) $(TARGET)

run-server: $(TARGET)
	./$(TARGET) server

run-client: $(TARGET)
	./$(TARGET) client

help:
	./$(TARGET) help
