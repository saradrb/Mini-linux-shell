#ifndef REDIRECTION_H
#define REDIRECTION_H

#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include "wildcard.h"

#include "external_commands.h"
#include "internal_commands.h"

/**
 * @brief check if the arg list contains a valid redirection (redirection symbol
 * and a filename destination)
 *
 * @param args_extanded argument array
 * @param size size of the array , number of args
 * @return return the position of the redirection symbol in array if finded
 * returns -1 if the redirection is invalid
 * returns -2 if there is no redirection
 */
int contains_valid_redirection(char** args_extanded, int size);

/**
 * @brief handle a command redirection
 *
 * @param redirection string that contains the redirection sign
 * @param filename the name of the destination file
 * @return return an array that contains the default descriptors of stdin dtdout
 * and stderr
 *
 */
int handle_redirection(char* redirection, char* filename);

// go back to standard descriptors for stdin stdout and stderr, after a
// redirection
void go_back_to_standard(int* fd_std);

// execute cmd with a redirection
int cmd_with_redirection(char* cmd, char** args, int length,
                         int pos_redirection);

#endif