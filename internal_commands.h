#include <ctype.h>
#include <readline/history.h>
#include <readline/readline.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define MAX_ARGS_NUMBER 4096
#define MAX_ARGS_STRLEN 4096
#define PATH_MAX 4096

/* Return the absolute path interpreted logically if there is no option or if
 * the option is -L, or interpreted physicaly if the option is -P
 */
int my_pwd(char **arguments, int length);

int my_cd(char **arguments, int length);

int my_exit(char **arguments, int length);