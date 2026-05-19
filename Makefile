CC=gcc
CFLAGS=-Wall -O2
LIBS=-lX11

all:
	$(CC) $(CFLAGS) main.c wm.c config.c -o arwm $(LIBS)