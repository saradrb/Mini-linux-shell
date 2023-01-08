#ifndef WILDCARD_H
#define WILDCARD_H

#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include "my_limits.h"

char *array_to_string(char **array_ofchar, int length, char *delemiter);

// free the memory allocated for the structure char**
void free_struct(char **my_struct, int size);

/**
 * @brief concatenate tab2 to tab1 starting at index position
 * @param tab1 first array
 * @param size1 length of  tab1
 * @param tab2 second array
 * @param size2 length of tab2
 * @param position index at which concat start inserting tab2
 * @return the fianl array tab3 with concatenation of tab1 and tab2 or return
 * tab1 if tab2 is empty
 */
char **concat(char **tab1, int size1, char **tab2, int size2, int position);

// concatenate to tab1 an element
char **concat_elem(char **tab1, int *size, char *elem);

// concatenate to tab2 to tab1
char **concat_tab(char **tab1, int *size, char **tab2, int size2);

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