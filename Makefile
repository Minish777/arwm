CC = gcc
CFLAGS = -Wall -O2
LIBS = -lX11
PREFIX = /usr/local

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
	mkdir -p $(DESTDIR)$(PREFIX)/share/xsessions
	echo "[Desktop Entry]\nName=ARWM\nComment=Another Robust Window Manager\nExec=$(PREFIX)/bin/arwm\nType=Application" > $(DESTDIR)$(PREFIX)/share/xsessions/arwm.desktop

uninstall:
	rm -f $(DESTDIR)$(PREFIX)/bin/$(TARGET)
	rm -f $(DESTDIR)$(PREFIX)/share/xsessions/arwm.desktop

clean:
	rm -f $(TARGET) $(OBJ)

.PHONY: all install uninstall clean
