#ifndef WILDCARD_H
#define WILDCARD_H

#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include "array.h"
#include "my_limits.h"

// parse a path with a delimiter
char **parse_path(char *path, int *length, char *delimiters);

int prefix(char *string, char *str, char *new_str);

int suffix(char *string, char *str, char *new_str);

// function that return in the array "options" all the path options after
// expansion of the * wildcard
int expand_star(char **path, int length, char *expanded_path, char **options,
                int *nb_options);

// function that checks if an array of char* contains an element that contains a
// wildcard
int contains_wildcard(char **tab_argument, int length, const char *wildcard);

// expand the ** joker, it take the path after the ** and search for it in the
// current repo tree
char **expand_double_star(char **path, int length, char *current_repo,
                          char **path_options, int *nb_op);

#endif