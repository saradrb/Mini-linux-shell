#include "array.h"

char *array_to_string(char **array_ofchar, int length, char *delemiter) {
  char *my_string = malloc(sizeof(char *));
  for (int i = 0; i < length; i++) {
    strcat(my_string, array_ofchar[i]);
    strcat(my_string, delemiter);
  }

  return my_string;
}

// char **char_array_concat(char **array1, char **array2) { return array1; }

// free the memory allocated for the structure char**
void free_struct(char **my_struct, int size) {
  for (int i = 0; i < size; i++) {
    free(my_struct[i]);
  }
  free(my_struct);
}

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
char **concat(char **tab1, int size1, char **tab2, int size2, int position) {
  char **tab3 = malloc(sizeof(char *) * (size1 + size2));
  int j = 0;
  if (size2 != 0) {
    for (int i = 0; i < position; i++) {
      tab3[i] = tab1[j];
      j++;
    }
    for (int i = position; i < size2 + position; i++) {
      tab3[i] = tab2[i - position];
    }
    if (position < size1 - 1) {
      j++;  // increment the index to jump position in tab1 (position of the
            // argument expanded)
      for (int i = size2 + position; i < size1 + size2 - 1; i++) {
        tab3[i] = tab1[j];
        j++;
      }
    }

  } else {
    for (int i = 0; i < size1; i++) {
      tab3[i] = tab1[i];
    }
  }
  tab3[size1 + size2 - 1] = NULL;
  return tab3;
}

// concatenate to tab1 an element
char **concat_elem(char **tab1, int *size, char *elem) {
  tab1 = realloc(tab1, sizeof(char *) * (*size + 2));
  tab1[*size] = malloc(sizeof(char) * PATH_MAX);
  sprintf(tab1[*size], "%s", elem);
  (*size)++;
  tab1[*size] = NULL;
  return tab1;
}

// concatenate to tab2 to tab1
char **concat_tab(char **tab1, int *size, char **tab2, int size2) {
  if (size2 != 0) {
    tab1 = realloc(tab1, sizeof(char *) * ((*size) + size2 + 1));
    for (int i = (*size); i < (*size) + size2; i++) {
      tab1[i] = tab2[i - (*size)];
    }
    tab1[(*size) + size2] = NULL;
    *size = (*size) + size2;
  }

  return (tab1);
}