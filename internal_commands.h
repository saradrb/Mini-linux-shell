#ifndef INTERNAL_COMMANDS_H
#define INTERNAL_COMMANDS_H
#include <ctype.h>
#include <dirent.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include "limits.h"

#define MAX_EXIT_VALUE 999

/* Exit the program with a certain value. If no value has been given, close
 * the program with the value the last function called has returned, otherwise
 * close it with the given value.
 * Return 1 if the given argument isn't valid (the program hasn't been closed)
 */
int my_exit(char** arguments, int length);

/* Write the absolute path interpreted logically if there is no option or if
 * the option is -L, or interpreted physicaly if the option is -P
 */
int my_pwd(char** arguments, int length);

/* Change the current working directory to another one. If no path is given,
 * change directory to the one described in 'HOME', if the path given is '-',
 * change directory to the previous one, and if a path is given, change
 * directory to the one described in path.
 * If no option is given, or if the option given is '-L', interprete the given
 * path logically. If the option given is '-P', interprete it physically.
 * Return 0 if the directory has been successfully changed and 1 otherwise.
 */
int my_cd(char** arguments, int length);

#endif
