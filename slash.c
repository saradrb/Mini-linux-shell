#include <readline/history.h>
#include <readline/readline.h>
#include "internal_commands.h"

extern char previous_rep[PATH_MAX];
extern char current_rep[PATH_MAX];
extern int previous_return_value;

/* Print prompt with color, containing the return value of
 * the last executed command and the path where we are
 */
static void my_prompt() {
  // convert the previous return value into a char[]
  char nbr[6] = {'\0'};
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
  if (len > 26 - strlen(nbr)) {
    strcat(res, "...");
    strcat(res, &current_rep[len - 22 + strlen(nbr) - 1]);
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
static char **split_line(char *line, char **cmd, int *length,
                         char *delimiters) {
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
static void read_cmd() {
  char *prompt_char = "$ ";
  int length = 0;
  char *cmd = " ";
  while (1) {
    my_prompt();
    // read the command line input
    char *line = readline(prompt_char);

    // if the file has ended, exit the program
    if (line == NULL) {
      write(STDOUT_FILENO, "\n", 1);
      my_exit(NULL, 0);
    }

    // if the line isnt empty
    if (line && *line) {
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
    }
  }
}

int main(int argc, char const *argv[]) {
  rl_outstream = stderr;
  getcwd(current_rep, PATH_MAX);
  read_cmd();
  return (0);
}
