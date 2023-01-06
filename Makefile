CC = gcc
CFLAGS = -Wall -g -pedantic
LDLIBS = -lreadline

slash : slash.o internal_commands.o signal.o
	$(CC) -o slash slash.o internal_commands.o signal.o $(LDLIBS) -Ilibrary

signal.o : signal.c signal.h
	$(CC) $(CFLAGS) -c -o signal.o signal.c
internal_commands.o : internal_commands.c internal_commands.h
	$(CC) $(CFLAGS) -c -o internal_commands.o internal_commands.c
slash.o : slash.c
	$(CC) $(CFLAGS) -c -o slash.o slash.c

clean :
	rm -f slash *.o