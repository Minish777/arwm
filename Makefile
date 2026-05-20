CC = gcc
CFLAGS = -Wall -O2
LIBS = -lX11
PREFIX = /usr/local

# Uncomment to enable debug logs
# CFLAGS += -DDEBUG

SRC = main.c wm.c config.c
OBJ = $(SRC:.c=.o)
TARGET = arwm

all: $(TARGET)

$(TARGET): $(OBJ)
	$(CC) $(CFLAGS) $(OBJ) -o $(TARGET) $(LIBS)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

install: $(TARGET)
	install -Dm755 $(TARGET) $(DESTDIR)$(PREFIX)/bin/$(TARGET)

uninstall:
	rm -f $(DESTDIR)$(PREFIX)/bin/$(TARGET)

clean:
	rm -f $(TARGET) $(OBJ)

.PHONY: all install uninstall clean
