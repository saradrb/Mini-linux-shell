#ifndef ARRAY_H
#define ARRAY_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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

#endif