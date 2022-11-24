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
#define EXIT_MAX_VALUE 255

char previous_rep[PATH_MAX] = {'\0'};
char current_rep[PATH_MAX] = {'\0'};

int previous_return_value = 0;

int my_exit(char **arguments, int length) {
  if (length > 1) {
    write(STDOUT_FILENO, "exit: Too many arguments\n", 26);
    return 1;
  }

  if (length == 0) {
    exit(previous_return_value);
  } else {
    for (int i = 0; i < strlen(arguments[0]); i++) {
      if (isdigit(arguments[0][i]) == 0) {
        write(STDOUT_FILENO, "exit: Invalid argument\n", 24);
        return 1;
      }
    }
    exit(atoi(arguments[0]));
  }
}

/* Search a directory in path between path[pos_path]
 * and the first next '/' or the end of path
 */
char *directory(char *path, int pos_path, int len) {
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

/* Suppress a directory at the end of path
 * (between path[pos] and a char '/')
 */
int suppr_directory(char *path, int pos) {
  do {
    path[pos] = '\0';
    pos -= 1;
  } while (path[pos] != '/');
  return pos;
}

/* Take an absolute path and return a simplification
 * of this path without the . and .. directory
 */
char *get_final_path(char *path) {
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
        pos_last_directory = suppr_directory(res, pos_last_directory + 1);
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

/* Test if the given argument is valid for the commands 'pwd' and 'cd'.
 * Return 0 if 'argument' is a valid option (-P or -L), -1 if it is any
 * other options and 0 if it isn't an option.
 * If the command given is 'cd', return 0 if the argument is '-'. */
static int is_valid_argument(char *argument, bool is_cd) {
  if (argument[0] == '-') {
    if (strcmp(argument, "-P") == 0) {
      return 0;
    } else if (strcmp(argument, "-L") == 0) {
      return 0;
    } else if (strcmp(argument, "-") == 0) {
      if (is_cd)
        return 1;
      else
        return -1;
    }
  }
  return 1;
}

/* Test if the given arguments are valid for the commands 'pwd' and 'cd'
 * and fill 'option' and 'path' with the correct argument (if more than one
 * valid option has been found, fill 'option' with the last one).
 * Return 0 if all given arguments are valid, -1 if an unknown option has been
 * found and 1 if more than one argument that isn't an option has been found. */
static int is_valid(char **arguments, int length, bool is_cd, char *option,
                    char *path) {
  for (int i = 0; i < length; i++) {
    int is_valid = is_valid_argument(arguments[i], is_cd);
    switch (is_valid) {
      case 0:
        strncpy(option, arguments[i], strlen(arguments[i]) + 1);
        break;
      case 1:
        if (!is_cd || strlen(path) != 0)
          return 1;  // there is more than one argument that isn't an option
        else
          strncpy(path, arguments[i], strlen(arguments[i]) + 1);
        break;
      default:
        return -1;  // there is an unknown option
    }
  }
  return 0;
}

/* Return the absolute path interpreted logically if there is no option or if
 * the option is -L, or interpreted physicaly if the option is -P
 */
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

int my_cd(char **arguments, int length) {
  char option[3] = {'\0'};
  char path[PATH_MAX] = {'\0'};

  int valid = is_valid(arguments, length, true, option, path);
  if (valid == 1) {
    write(STDOUT_FILENO, "cd: Too many arguments\n", 24);
    return 1;
  }
  if (valid == -1) {
    write(STDOUT_FILENO, "cd: Unknown option\n", 20);
    return 1;
  }

  if (strlen(path) == 0) {
    char *home = getenv("HOME");
    if (chdir(home) == -1) {
      write(STDOUT_FILENO, "cd: Invalid path\n", 18);
      return 1;
    }
    strncpy(previous_rep, current_rep, PATH_MAX);
    strncpy(current_rep, getenv("HOME"), strlen(home) + 1);
    return 0;
  }

  if (strcmp(path, "-") == 0) {
    if (previous_rep == NULL) {
      write(STDOUT_FILENO, "cd: No previous directory\n", 27);
      return 1;
    }
    if (chdir(previous_rep) == -1) {
      write(STDOUT_FILENO, "cd: Invalid path\n", 18);
      return 1;
    }
    char tmp[PATH_MAX];
    strncpy(tmp, previous_rep, PATH_MAX);
    strncpy(previous_rep, current_rep, PATH_MAX);
    strncpy(current_rep, tmp, PATH_MAX);
    return 0;
  }

  char tmp[PATH_MAX] = {'\0'};
  if (strlen(option) == 0 || strcmp(option, "-L") == 0) {
    if (path[0] != '/') {
      int length = strlen(current_rep);
      strncpy(tmp, current_rep, PATH_MAX);
      strncpy(tmp + length, "/", 2);
      strncpy(tmp + length + 1, path, strlen(path) + 1);
      char *final_path = get_final_path(tmp);
      strncpy(tmp, final_path, strlen(final_path) + 1);
      free(final_path);
    }
  } else {
    strncpy(tmp, path, strlen(path) + 1);
  }

  if (chdir(tmp) == -1) {
    if (strlen(option) == 0 || strcmp(option, "-L") == 0) {
      if (chdir(path) == -1) {
        write(STDOUT_FILENO, "cd: Invalid path\n", 18);
        return 1;
      }
      strncpy(previous_rep, current_rep, PATH_MAX);
      getcwd(current_rep, PATH_MAX);
      return 0;
    } else {
      write(STDOUT_FILENO, "cd: Invalid path\n", 18);
      return 1;
    }
  }
  strncpy(previous_rep, current_rep, PATH_MAX);
  if (strcmp(option, "-P") == 0)
    getcwd(current_rep, PATH_MAX);
  else
    strncpy(current_rep, tmp, PATH_MAX);
  /*
  if (strcmp(option, "-P") == 0) {
    getcwd(current_rep, PATH_MAX);
  } else {*/

  /*
  if (path[0] == '/') {
    strncpy(current_rep, path, strlen(path) + 1);
  } else {
    int length = strlen(current_rep);
    strncpy(current_rep + length, "/", 2);
    strncpy(current_rep + length + 1, path, strlen(path) + 1);
    char *final_path = get_final_path(current_rep);
    strncpy(current_rep, final_path, strlen(final_path) + 1);
    free(final_path);
  }*/

  return 0;
}

/* Print prompt with color, containing the return value of
 * the last executed command and the path where we are
 */
void my_prompt() {
  // convert the previous return value into a char[]
  char nbr[12];
  sprintf(nbr, "%d", previous_return_value);

  char res[50] = {'\0'};
  // complete res with green if the last return value is 0, and with red
  // otherwise
  if (previous_return_value == 0) {
    char *start_prompt_green = "\001\033[32m\002[";
    strcat(res, start_prompt_green);
  } else {
    char *start_prompt_red = "\001\033[91m\002[";
    strcat(res, start_prompt_red);
  }
  strcat(res, nbr);

  // change the color to cyan
  char *start_pwd_cyan = "]\001\033[36m\002";
  strcat(res, start_pwd_cyan);

  char *end_prompt = "\001\033[00m\002";
  int len = strlen(current_rep);
  // complete res with the absolute path, or with a simplification of the
  // absolute path if length of path is more then 25 characters
  if (len > 25) {
    strcat(res, "...");
    strcat(res, &current_rep[len - 22]);
  } else {
    strcat(res, current_rep);
  }
  // change the color to default and write res into STDERR
  strcat(res, end_prompt);
  if (write(STDERR_FILENO, res, strlen(res)) != strlen(res)) {
    return;
  }
}

// function that takes the input line and parse it, it returns an array of args
// , number of args and the command
char **split_line(char *line, char **cmd, int *length, char *delimiters) {
  char **list_arg = NULL;           // allocate memory for the args array
  *cmd = strtok(line, delimiters);  // the first arg is the command
  int nb_spaces = 0;
  // parse the line into args
  char *arg = strtok(NULL, " ");

  while (arg && (nb_spaces < MAX_ARGS_NUMBER)) {
    nb_spaces++;
    list_arg = realloc(list_arg, sizeof(char *) * nb_spaces);
    if (list_arg == NULL) exit(-1);  // memory allocation failed
    list_arg[nb_spaces - 1] = arg;
    arg = strtok(NULL, " ");
  }
  list_arg = realloc(list_arg, sizeof(char *) * (nb_spaces + 1));
  list_arg[nb_spaces] = NULL;

  *length = nb_spaces;
  return list_arg;
}

// function that read the command line input and format it
void read_cmd() {
  char *prompt_char = "$ ";
  int length = 0;
  char *cmd = " ";
  while (1) {
    my_prompt();
    // read the command line input
    char *line = readline(prompt_char);
    if (line && *line) {  // if the line isnt empty

      add_history(line);

      // split the line
      char **list_arg = split_line(line, &cmd, &length, " ");

      // exec command
      if (strcmp(cmd, "exit") == 0) {
        previous_return_value = my_exit(list_arg, length);
      } else {
        if (strcmp(cmd, "cd") == 0) {
          previous_return_value = my_cd(list_arg, length);
        } else {
          if (strcmp(cmd, "pwd") == 0) {
            previous_return_value = my_pwd(list_arg, length);
          } else {
            printf("%s: command not found\n", cmd);
            previous_return_value = 1;
          }
        }
      }

      // free the memory allocated
      free(list_arg);
      free(line);
    } else {
      previous_return_value = my_exit(NULL, 0);
    }
  }
}

int main(int argc, char const *argv[]) {
  rl_outstream = stderr;
  getcwd(current_rep, PATH_MAX);
  read_cmd();
  return (0);
}
