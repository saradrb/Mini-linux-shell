#include "internal_commands.h"

char previous_rep[PATH_MAX] = {'\0'};
char current_rep[PATH_MAX] = {'\0'};
int previous_return_value = 0;

/* Exit the program with a certain value. If no value has been given, close
 * the program with the value the last function called has returned, otherwise
 * close it with the given value.
 * Return 1 if the given argument isn't valid (the program hasn't been closed) */
int my_exit(char **arguments, int length) {
  if (length > 1) {
    write(STDOUT_FILENO, "exit: Too many arguments\n", 26);
    return 1;
  }

  // if no argument has been given, exit with the previous return value
  if (length == 0) {
    exit(previous_return_value);
  } else {
    // verify if the given argument is a number
    for (int i = 0; i < strlen(arguments[0]); i++) {
      if (isdigit(arguments[0][i]) == 0) {
        write(STDOUT_FILENO, "exit: Invalid argument\n", 24);
        return 1;
      }
    }
    int value = atoi(arguments[0]);
    if (value > MAX_EXIT_VALUE) {
        write(STDOUT_FILENO, "exit: Exit value too big\n", 26);
        return 1;
    }
    exit(atoi(arguments[0]));
  }
}

/* Search a directory in path between path[pos_path] and the first next '/' or
 * the end of path */
static char *directory(char *path, int pos_path, int len) {
  // allocation and management of the memory
  char *word = malloc(sizeof(char) * (len - pos_path + 2));
  if (word == NULL) {
    free(word);
    return NULL;
  }

  // search the directory
  int pos_word = 0;
  while (path[pos_path] != '\0' && path[pos_path] != '/') {
    word[pos_word] = path[pos_path];
    pos_path += 1;
    pos_word += 1;
  }

  // add the null byte
  word[pos_word] = '\0';
  return word;
}

/* Delete a directory at the end of path (between path[pos] and a char '/') */
static int delete_directory(char *path, int pos) {
  do {
    path[pos] = '\0';
    pos -= 1;
  } while (path[pos] != '/');
  return pos;
}

/* Take an absolute path and return a simplification of this path without the
 * . and .. directory */
static char *get_final_path(char *path) {
  // verify if the first character is '/' (root)
  if (path[0] != '/') {
    return NULL;
  }

  // allocation and management of the memory
  char *res = malloc(sizeof(char) * (strlen(path) + 2));
  if (res == NULL) {
    free(res);
    return NULL;
  }

  // fill res with '\0' (to use strlen(word))
  for (int i = 0; i < strlen(path) + 2; i++) {
    res[i] = '\0';
  }

  res[0] = '/';
  int len = strlen(path);
  int pos_last_directory = 0;
  bool slash = false;
  for (int i = 1; i < strlen(path); i++) {
    // skim through path, directory by directory
    char *word = directory(path, i, len);

    // when word is .. suppress the last directory added in res except if the
    // directory is root
    if (strcmp(word, "..") == 0) {
      if (pos_last_directory != 0) {
        pos_last_directory = delete_directory(res, pos_last_directory + 1);
        if (pos_last_directory > 0) pos_last_directory -= 1;
        i += 2;
      }
      slash = false;
      // when world is . ignore word and the next '/'
    } else if (strcmp(word, ".") == 0) {
      i += 1;
      slash = false;
      // when word is a simple directory, add this directory to res
    } else if (strlen(word) != 0) {
      strcat(res, word);
      i += strlen(word) - 1;
      pos_last_directory = strlen(res) - 1;
      slash = false;
      // when path contain two '/' following each other, release the memory
      // and return NULL
    } else if (slash == true) {
      free(res);
      free(word);
      return NULL;
      // when word is equal to '/', add word in res
    } else {
      res[pos_last_directory + 1] = path[i];
      slash = true;
    }
    // release memory
    free(word);
  }
  // add the null byte
  res[pos_last_directory + 1] = '\0';
  return res;
}

/* Get the given argument type for the commands 'pwd' and 'cd'.
 * Return 0 if 'argument' is a valid option (-P or -L), -1 if it is any
 * other options and 1 if the argument is '-', or isn't an option. */
static int get_argument_type(char *argument, bool is_cd) {
  if (argument[0] == '-') {  // the argument is an option
    if (strcmp(argument, "-P") == 0 || strcmp(argument, "-L") == 0) return 0;
    if (is_cd && strcmp(argument, "-") == 0) return 1;
    return -1;
  }
  return 1;  // the argument isn't an option
}

/* Test if the given arguments are valid for the commands 'pwd' and 'cd'
 * and fill 'option' (and 'path' if 'is_cd' is true) with the correct
 * argument (if more than one valid option has been found, fill 'option' with
 * the last one).
 * Return 0 if all given arguments are valid, -1 if an unknown option has been
 * found and 1 if more than the valid number or arguments has been found. */
static int is_valid(char **arguments, int length, bool is_cd, char *option,
                    char *path) {
  for (int i = 0; i < length; i++) {
    int type = get_argument_type(arguments[i], is_cd);
    switch (type) {
      case 0:  // the argument is a valid option
        strncpy(option, arguments[i], strlen(arguments[i]) + 1);
        break;
      case 1:  // the argument isn't an option
        if (!is_cd || strlen(path) != 0)
          return 1;  // there is more than the valid number of argument
        strncpy(path, arguments[i], strlen(arguments[i]) + 1);
        break;
      default:
        return -1;  // there is an unknown option
    }
  }
  return 0;
}

/* Write the absolute path interpreted logically if there is no option or if
 * the option is -L, or interpreted physicaly if the option is -P */
int my_pwd(char **arguments, int length) {
  char option[3] = {'\0'};
  // check if the arguments are valid
  int valid = is_valid(arguments, length, false, option, NULL);
  if (valid == 1) {
    write(STDOUT_FILENO, "pwd: Too many arguments\n", 24);
    return 1;
  }
  // check if the options are valid
  if (valid == -1) {
    write(STDOUT_FILENO, "pwd: Unknown option\n", 20);
    return 1;
  }

  char buffer[PATH_MAX] = {'\0'};
  // if there is a -P option : return the physical interpretation with getcwd
  if (strcmp(option, "-P") == 0) {
    getcwd(buffer, sizeof(buffer));
    strcat(buffer, "\n");
    write(STDOUT_FILENO, buffer, strlen(buffer));
    return 0;
    // if the option isn't -P, return the logical interpretation
    // with the global variable current repertory
  } else {
    strcat(buffer, current_rep);
    strcat(buffer, "\n");
    write(STDOUT_FILENO, buffer, strlen(buffer));
    return 0;
  }

  return 1;
}

/* Change the current working directory to another one. If no path is given,
 * change directory to the one described in 'HOME', if the path given is '-',
 * change directory to the previous one, and if a path is given, change
 * directory to the one described in path.
 * If no option is given, or if the option given is '-L', interprete the given
 * path logically. If the option given is '-P', interprete it physically.
 * Return 0 if the directory has been successfully changed and 1 otherwise.
*/
int my_cd(char **arguments, int length) {
  char option[3] = {'\0'};
  char path[PATH_MAX] = {'\0'};
  char new_current[PATH_MAX] = {'\0'};
  bool physical_interpretation = false;

  // verify the validity of the arguments
  int valid = is_valid(arguments, length, true, option, path);
  if (valid == 1) {
    write(STDOUT_FILENO, "cd: Too many arguments\n", 24);
    return 1;
  }
  if (valid == -1) {
    write(STDOUT_FILENO, "cd: Unknown option\n", 20);
    return 1;
  }

  // stock the HOME directory in 'new_current'
  if (strlen(path) == 0) {
    char *home = getenv("HOME");
    strncpy(new_current, home, strlen(home) + 1);
  }

  // stock the previous directory in 'new_current'
  else if (strcmp(path, "-") == 0) {
    if (previous_rep == NULL) {  // there is no previous directory
      write(STDOUT_FILENO, "cd: No previous directory\n", 27);
      return 1;
    }
    strncpy(new_current, previous_rep, PATH_MAX);
  }

  // stock the directory specified in 'path' (interpreted logically by removing
  // '.' and '..') in 'new_current'
  else if (strlen(option) == 0 || strcmp(option, "-L") == 0) {
    if (path[0] == '/') {  // path is an absolute reference, just copy it
      strncpy(new_current, path, strlen(path) + 1);
    } else {  // path isn't an absolute reference, transform it and copy it
      int current_rep_length = strlen(current_rep);
      strncpy(new_current, current_rep, PATH_MAX);
      // add a '/' after copying 'current_rep', unless 'current_rep' is just '/'
      if (strcmp(current_rep, "/") != 0) {
        strncpy(new_current + current_rep_length, "/", 2);
        current_rep_length++;
      }
      strncpy(new_current + current_rep_length, path, strlen(path) + 1);
    }
    // remove all '.' and '..' that can be found and transform the path
    // correspondingly
    char *final_path = get_final_path(new_current);
    strncpy(new_current, final_path, strlen(final_path) + 1);
    free(final_path);
  }

  // stock the directory specified in 'path' (interpreted physically by chdir
  // itself) in 'new_current'
  else {
    physical_interpretation = true;
    strncpy(new_current, path, strlen(path) + 1);
  }

  // change the current directory to the one specified in 'new_current'
  if (chdir(new_current) == -1) {
    // if the path is invalid and it was a logical interpretation, interprete it
    // physically
    if (!physical_interpretation) {
      physical_interpretation = true;
      strncpy(new_current, path, strlen(path) + 1);
      if (chdir(new_current) == -1) {
        write(STDOUT_FILENO, "cd: Invalid path\n", 18);
        return 1;
      }
    } else {
      write(STDOUT_FILENO, "cd: Invalid path\n", 18);
      return 1;
    }
  }

  // fill the global variables 'previous_rep' and 'current_rep'
  strncpy(previous_rep, current_rep, PATH_MAX);
  if (physical_interpretation)
    getcwd(current_rep, PATH_MAX);
  else
    strncpy(current_rep, new_current, PATH_MAX);

  return 0;
}