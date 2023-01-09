#include <errno.h>
#include <fcntl.h>
#include <readline/history.h>
#include <readline/readline.h>
#include <stdbool.h>
#include <sys/wait.h>

#include "external_commands.h"
#include "internal_commands.h"
#include "library/string.c"
#include "my_limits.h"
#include "pipeline.h"
#include "redirection.h"
#include "signal.h"
#include "wildcard.h"

extern char previous_rep[PATH_MAX];
extern char current_rep[PATH_MAX];
extern int previous_return_value;

static char **copy_part_of_cmd(char **cmd, int start, int finish) {
  char **res = malloc((finish + 2 - start) * sizeof(char *));
  if (res == NULL) {
    perror("malloc");
    free(res);
  }

  for (int i = start; i <= finish; i++) {
    char *string = malloc(strlen(cmd[i]) + 1);
    if (string == NULL) {
      perror("malloc");
      free(string);
    }

    strncpy(string, cmd[i], strlen(cmd[i]) + 1);
    res[i - start] = string;
  }

  res[finish + 1 - start] = NULL;
  return res;
}

static void free_triple_tab(char ***tab) {
  int i = 0;
  while (tab[i] != NULL) {
    int j = 0;
    char **subarray = tab[i];
    while (subarray[j] != NULL) {
      free(subarray[j]);
      j++;
    }
    free(subarray);
    i++;
  }
  free(tab);
}

char ***split_cmd_to_pipeline(char **cmd, int nbr_of_pipes) {
  char ***res = malloc((nbr_of_pipes + 2) * sizeof(char **));
  if (res == NULL) {
    perror("malloc");
    free(res);
  }

  int i = 0;
  int prev = 0;
  int j = 0;
  while (cmd[j] != NULL) {
    if (strcmp(cmd[j], "|") == 0) {
      res[i] = copy_part_of_cmd(cmd, prev, j - 1);
      i = i + 1;
      prev = j + 1;
      j = j + 1;
    } else {
      j = j + 1;
    }
  }
  res[i] = copy_part_of_cmd(cmd, prev, j - 1);
  res[i + 1] = NULL;
  return res;
}

int test_tab(char ***tab) {
  int i = 0;
  while (tab[i] != NULL) {
    int j = 0;
    char **tmp = tab[i];
    if (tmp[j] == NULL) {
      return 55;
    }
    if (j == 0 && (strcmp(tmp[j], "<") == 0 || strcmp(tmp[j], ">") == 0 ||
                   strcmp(tmp[j], ">|") == 0 || strcmp(tmp[j], ">>") == 0 ||
                   strcmp(tmp[j], "2>") == 0 || strcmp(tmp[j], "2>>") == 0 ||
                   strcmp(tmp[j], "2>|") == 0)) {
      return 55;
    }
    while (tmp[j] != NULL) j++;
    if (j != 0 &&
        (strcmp(tmp[j - 1], "<") == 0 || strcmp(tmp[j - 1], ">") == 0 ||
         strcmp(tmp[j - 1], ">|") == 0 || strcmp(tmp[j - 1], ">>") == 0 ||
         strcmp(tmp[j - 1], "2>") == 0 || strcmp(tmp[j - 1], "2>>") == 0 ||
         strcmp(tmp[j - 1], "2>|") == 0)) {
      return 55;
    }
    i++;
  }
  return 0;
}

int nbr_pipe(char **tab) {
  int i = 0;
  int len = 0;
  while (tab[i] != NULL) {
    if (strcmp(tab[i], "|") == 0) {
      len++;
    }
    i++;
  }
  return len;
}

/**
 * @brief Print prompt with color, containing the return value of the last
 * executed command and the path where we are
 */
static void my_prompt() {
  // convert the previous return value into a char[]
  char nbr[6] = {'\0'};
  if (previous_return_value == 255)
    strncpy(nbr, "SIG", 4);
  else
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
  // absolute path if length of path is more then 26 characters
  if (len > 26 - strlen(nbr)) {
    strcat(res, "...");
    strcat(res, &current_rep[len - 23 + strlen(nbr)]);
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

/**
 * @brief function that read the command line input and format it
 */
static void read_cmd() {
  char *prompt_char = "$ ";
  int length = 0;
  char *cmd = " ";
  int nb_parts = 0;

  while (1) {
    my_prompt();
    // read the command line input
    char *line = readline(prompt_char);
    char *trimmed_line = trim(line);

    // if the file has ended, exit the program
    if (trimmed_line == NULL) {
      write(STDOUT_FILENO, "\n", 1);
      my_exit(NULL, 0);
    }

    // if line is empty then don't do anything
    if (!(trimmed_line && *trimmed_line)) continue;

    add_history(trimmed_line);

    // split the line
    char **list_arg = split_line(trimmed_line, &cmd, &length, " ");

    // test if there is an argument that contains a wildcard (* or **)
    int size = 0;
    char **args_extanded = NULL;
    int nb_cmds = 0;
    if (strstr(cmd, "*")) {
      char *expanded_path = malloc(sizeof(char) * PATH_MAX);
      if (expanded_path == NULL) {
        perror("malloc");
        free(expanded_path);
      }

      memset(expanded_path, 0, sizeof(char) * PATH_MAX);
      char **mypath = parse_path(cmd, &nb_parts, "/");
      char **new_cmd = malloc(sizeof(char *) * 10);
      if (new_cmd == NULL) {
        perror("malloc");
        free(new_cmd);
      }

      expand_star(mypath, nb_parts, expanded_path, new_cmd, &nb_cmds);
      free(mypath);
      free(expanded_path);

      switch (nb_cmds) {
        case 0:
          args_extanded = concat_elem(args_extanded, &size, cmd);
        default:
          args_extanded = concat_tab(args_extanded, &size, new_cmd, nb_cmds);
          cmd = args_extanded[0];
      }
      free(new_cmd);
    } else {
      args_extanded = concat_elem(args_extanded, &size, cmd);
    }

    for (int i = 0; i < length; i++) {
      if (prefix("**", list_arg[i], NULL)) {
        // if it contains a ** return all the possible options in the current
        // repo tree
        char **mypath = parse_path(list_arg[i], &nb_parts, "/");
        args_extanded = expand_double_star(mypath + 1, nb_parts - 1, "",
                                           args_extanded, &size);

        // if we dont find any option concatenate arg as it is
        if (size == 0) {
          args_extanded = concat_elem(args_extanded, &size, list_arg[i]);
        }
        free(mypath);

      } else {
        if (strstr(list_arg[i], "*") && !strstr(list_arg[i], "**")) {
          // then expand the star and return an array with all the path
          // options
          char *expanded_path = malloc(sizeof(char) * PATH_MAX);
          if (expanded_path == NULL) {
            perror("malloc");
            free(expanded_path);
          }
          memset(expanded_path, 0, sizeof(char) * PATH_MAX);

          char **mypath = parse_path(list_arg[i], &nb_parts, "/");
          int nb_options = 0;
          char **options = malloc(sizeof(char *) * PATH_MAX);
          if (options == NULL) {
            perror("malloc");
            free(options);
          }

          expand_star(mypath, nb_parts, expanded_path, options, &nb_options);
          switch (nb_options) {
            case 0:
              args_extanded = concat_elem(args_extanded, &size, list_arg[i]);
            default:
              // concat final array to the argument array
              args_extanded =
                  concat_tab(args_extanded, &size, options, nb_options);
          }
          // free the allocated memory
          free(options);
          free(mypath);
          free(expanded_path);
        } else {
          // concat elem to the arg array
          args_extanded = concat_elem(args_extanded, &size, list_arg[i]);
        }
      }
    }  // expanding the path

    // exec command

    // handle redirection if any
    int pos_redirection = contains_valid_redirection(args_extanded, size);
    int nbr_of_pipes = nbr_pipe(args_extanded);
    if (nbr_of_pipes > 0) {
      char ***pipeline = split_cmd_to_pipeline(args_extanded, nbr_of_pipes);
      if (pipeline == NULL) {
        perror("error");
      }
      if (test_tab(pipeline) == 0) {
        int n = exec_pipeline(pipeline);
        previous_return_value = n;
        // free(pipeline);
      } else {
        // free(pipeline);
        previous_return_value = 2;
      }
      free_triple_tab(pipeline);

    } else if (pos_redirection > 0 || pos_redirection == 0) {
      previous_return_value =
          cmd_with_redirection(cmd, args_extanded, size, pos_redirection);
    } else if (strcmp(cmd, "exit") == 0) {
      previous_return_value = my_exit(args_extanded + 1, length);
    } else if (strcmp(cmd, "cd") == 0) {
      previous_return_value = my_cd(args_extanded + 1, length);
    } else if (strcmp(cmd, "pwd") == 0) {
      previous_return_value = my_pwd(args_extanded + 1, length);
    } else {
      // execute external command with the new extanded array of args
      previous_return_value = extern_command(cmd, args_extanded);
    }

    free_struct(args_extanded, size);
    free(list_arg);
    free(line);
    free(trimmed_line);
  }
}

int main(int argc, char const *argv[]) {
  rl_outstream = stderr;
  getcwd(current_rep, PATH_MAX);
  treat_signal(true);
  read_cmd();
  return (0);
}