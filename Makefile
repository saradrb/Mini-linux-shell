CC = gcc
CFLAGS = -Wall -g -pedantic
LDLIBS = -lreadline

slash : slash.c
	$(CC) $(CFLAGS) slash.c -o slash $(LDLIBS)

clean :
	rm -f slash