#ifndef EXTERNAL_COMMANDS_H
#define EXTERNAL_COMMANDS_H

#include <stdbool.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include "signal.h"

/**
 * @brief Execute every external command with or with out argument
 *
 * @param cmd command
 * @param args array containing the command and his arguments
 * @return the return value of the execution of the command : 0 for successed, 1
 * if exec failed, 8 if exec successed but the command or argument is wrong
 */
int extern_command(char *cmd, char **args);

int extern_command_bis(char *cmd, char **args, int fd_write[], int fd_read[]);

#endif