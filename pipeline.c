#include "pipeline.h"

/**
 * @brief verify if there is a valid redirection and call the method
 * cmd_with_redirection_pipe if there is one, or call the method exec_cmd
 *
 * @param first_last 0 if it's the first command of pipe, 1 if it's the last
 * command of pipe
 * @param fd_write writing pipe
 * @param fd_read reading pipe
 * @param cmd array containing command ands arguments
 * @param mid 0 if it's a middle command of pipelines
 * @return value of the executed command
 */
int exec_first_last_command(int first_last, int fd_write[], int fd_read[],
                            char **cmd, int mid) {
  int pos_redirection = 0;
  int length = 0;
  int nbr_redirection = 0;
  int previous_return_value = 0;
  // count number of redirections and take the position of the first one
  for (int i = 0; cmd[i] != NULL; i++) {
    length = length + 1;
    if (strcmp(cmd[i], "<") == 0 || strcmp(cmd[i], ">") == 0 ||
        strcmp(cmd[i], ">|") == 0 || strcmp(cmd[i], ">>") == 0 ||
        strcmp(cmd[i], "2>") == 0 || strcmp(cmd[i], "2>>") == 0 ||
        strcmp(cmd[i], "2>|") == 0) {
      pos_redirection = i;
      nbr_redirection = nbr_redirection + 1;
    }
  }
  // check if it's a valid redirection
  if (((nbr_redirection <= 2 && nbr_redirection > 0) &&
       (((strcmp(cmd[pos_redirection], "<") == 0 ||
          strcmp(cmd[pos_redirection], "2>") == 0 ||
          strcmp(cmd[pos_redirection], "2>>") == 0 ||
          strcmp(cmd[pos_redirection], "2>|") == 0) &&
         first_last == 0) ||
        ((strcmp(cmd[pos_redirection], ">") == 0 ||
          strcmp(cmd[pos_redirection], ">|") == 0 ||
          strcmp(cmd[pos_redirection], ">>") == 0 ||
          strcmp(cmd[pos_redirection], "2>") == 0 ||
          strcmp(cmd[pos_redirection], "2>>") == 0 ||
          strcmp(cmd[pos_redirection], "2>|") == 0) &&
         first_last == 1))) ||
      (mid == 1 && (strcmp(cmd[pos_redirection], "2>") == 0 ||
                    strcmp(cmd[pos_redirection], "2>>") == 0 ||
                    strcmp(cmd[pos_redirection], "2>|") == 0))) {
    // call the method cmd_with_redirection_pipe to redirect and execute the
    // command
    previous_return_value = cmd_with_redirection_pipe(
        cmd[0], cmd, length, pos_redirection, fd_write, fd_read);
    return previous_return_value;

  } else {
    // call extern_command_bis to execute the command
    previous_return_value = extern_command_bis(cmd[0], cmd, fd_write, fd_read);
    return previous_return_value;
  }
}

/**
 * @brief return value of the length
 *
 * @param tab array of commands
 * @return length of tab
 */
int length_of_pipelines(char ***tab) {
  int length = 0;
  for (int i = 0; tab[i] != NULL; i++) {
    length = length + 1;
  }
  return length;
}

/**
 * @brief
 *
 * @param tab array of commands
 * @return value of the execution of pipelines
 */
int exec_pipeline(char ***tab) {
  int length = length_of_pipelines(tab);
  int fake_pipe[2] = {-1, -1};
  // create array of pipes
  int pipes[length - 1][2];

  // initialization of pipes
  for (int i = 0; i < length - 1; i++) {
    if (pipe(pipes[i]) == -1) return -2;
  }

  // value of the execution of the first command
  int previous_return_value =
      exec_first_last_command(0, pipes[0], fake_pipe, tab[0], 0);
  // manage the case if the command fail
  if (previous_return_value != 0) {
    close(pipes[0][1]);
    // close rest of pipes
    for (int i = 0; i < length - 1; i++) {
      close(pipes[i][1]);
      close(pipes[i][0]);
    }
    return previous_return_value;
  }
  for (int i = 1; i < length - 1; i++) {
    // execute every middle commands in pipelines
    previous_return_value =
        exec_first_last_command(-1, pipes[i], pipes[i - 1], tab[i], 1);

    // manage the case if the command fail
    if (previous_return_value != 0) {
      close(pipes[i][0]);
      // close rest of pipes
      for (int j = i + 1; j < length - 1; j++) {
        close(pipes[j][1]);
        close(pipes[j][0]);
      }
      return previous_return_value;
    }
  }

  // return the value of execution of the first command
  previous_return_value = exec_first_last_command(
      1, fake_pipe, pipes[length - 2], tab[length - 1], 0);

  return previous_return_value;
}