CC = gcc
CFLAGS = -Wall -Wextra -Wpedantic -std=c17 -Iinclude
LIBS = $(shell pkg-config --cflags --libs xcb xcb-ewmh xcb-icccm xcb-keysyms xcb-util)
PREFIX ?= /usr/local

SRC = src/main.c src/wm.c src/events.c src/ipc.c src/layout.c src/config.c src/ewmh.c src/keybinds.c src/monitor.c src/client.c src/util.c
OBJ = $(SRC:.c=.o)
TARGET = arwm
MSG_TARGET = arwm-msg

all: $(TARGET) $(MSG_TARGET)

$(TARGET): $(OBJ)
	$(CC) $(CFLAGS) $(OBJ) -o $(TARGET) $(LIBS)

$(MSG_TARGET): src/arwm-msg.o
	$(CC) $(CFLAGS) src/arwm-msg.o -o $(MSG_TARGET)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

debug: CFLAGS += -g -O0 -fsanitize=address,undefined
debug: all

install: all
	sudo ./scripts/install.sh
	sudo install -Dm755 $(MSG_TARGET) $(PREFIX)/bin/$(MSG_TARGET)

uninstall:
	sudo ./scripts/uninstall.sh
	sudo rm -f $(PREFIX)/bin/$(MSG_TARGET)

clean:
	rm -f src/*.o $(TARGET) $(MSG_TARGET)

.PHONY: all clean install uninstall debug
