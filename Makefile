CC = gcc
CFLAGS = -Wall -g -pedantic
LDLIBS = -lreadline

slash : slash.o internal_commands.o signal.o redirection.o external_commands.o wildcard.o pipeline.o
	$(CC) -o slash slash.o internal_commands.o signal.o redirection.o external_commands.o wildcard.o pipeline.o $(LDLIBS) -Ilibrary

signal.o : signal.c signal.h
	$(CC) $(CFLAGS) -c -o signal.o signal.c
redirection.o : redirection.c redirection.h
	$(CC) $(CFLAGS) -c -o redirection.o redirection.c
wildcard.o : wildcard.c wildcard.h
	$(CC) $(CFLAGS) -c -o wildcard.o wildcard.c
internal_commands.o : internal_commands.c internal_commands.h
	$(CC) $(CFLAGS) -c -o internal_commands.o internal_commands.c
external_commands.o : external_commands.c external_commands.h
	$(CC) $(CFLAGS) -c -o external_commands.o external_commands.c
pipeline.o : pipeline.c pipeline.h
	$(CC) $(CFLAGS) -c -o pipeline.o pipeline.c
slash.o : slash.c
	$(CC) $(CFLAGS) -c -o slash.o slash.c

clean :
	rm -f slash *.o