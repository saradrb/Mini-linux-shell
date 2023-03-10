#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/**
 * @brief Tests if the given string contains only whitespace
 *
 * @param string a string
 * @return true the given string contains only whitespace
 * @return false the given string contains other characters
 */
static bool only_whitespace(const char* string) {
  for (int i = 0; i < strlen(string); i++) {
    if (string[i] != ' ') return false;
  }
  return true;
}

/**
 * @brief Return a trimmed version of the given string
 *
 * @param string a string
 * @return char* an allocated trimmed version
 */
char* trim(const char* string) {
  if (string == NULL) return NULL;

  char* trimmed = malloc(sizeof(char) * (strlen(string) + 1));
  if (trimmed == NULL) {
    free(trimmed);
    return NULL;
  }

  bool content_started = false;
  int trimmed_index = 0;
  for (int i = 0; i < strlen(string); i++) {
    if (!content_started) {
      if (string[i] != ' ') content_started = true;
    }

    if (content_started) {
      if (string[i] == ' ' && only_whitespace(string + i)) {
        return trimmed;
      } else {
        trimmed[trimmed_index] = string[i];
        trimmed_index++;
      }
    }
  }

  trimmed[trimmed_index] = '\0';
  return trimmed;
}