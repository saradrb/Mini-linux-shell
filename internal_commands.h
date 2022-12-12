#ifndef _INTERNALCOMMANDS_H_
#define _INTERNALCOMMANDS_H_
#include <ctype.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdio.h>
#define MAX_ARGS_NUMBER 4096
#define MAX_ARGS_STRLEN 4096
#define PATH_MAX 4096
#define MAX_EXIT_VALUE 999

/* Exit the program with a certain value. If no value has been given, close
 * the program with the value the last function called has returned, otherwise
 * close it with the given value.
 * Return 1 if the given argument isn't valid (the program hasn't been closed)
*/
int my_exit(char **arguments, int length);

/* Write the absolute path interpreted logically if there is no option or if
 * the option is -L, or interpreted physicaly if the option is -P
 */
int my_pwd(char **arguments, int length);

/* Change the current working directory to another one. If no path is given,
 * change directory to the one described in 'HOME', if the path given is '-',
 * change directory to the previous one, and if a path is given, change
 * directory to the one described in path.
 * If no option is given, or if the option given is '-L', interprete the given
 * path logically. If the option given is '-P', interprete it physically.
 * Return 0 if the directory has been successfully changed and 1 otherwise.
*/
int my_cd(char **arguments, int length);

// function that return in the array "options" all the path options after expansion of the * wildcard
int expand_star(char** path,int length,char* expanded_path,char** options,int*nb_options);

//parse a path with a delimiter
char **parse_path(char *path, int *length, char *delimiters);

//free the memory allocated for the structure char**
void free_struct(char **my_struct,int size);

// function that checks if an array of char* contains an element that contains a wildcard 
int contains_wildcard(char ** tab_argument,int length,const char* wildcard);

// function that concatenate tab2 to tab1 starting at index position
char ** concat(char** tab1,int size1,char** tab2,int size2,int position);

//concatenate to tab1 an element 
char** concat_elem(char** tab1,int *size,char* elem);

//concatenate to tab2 to tab1
char** concat_tab(char **tab1,int *size,char** tab2, int size2);

#endif
