#include "external_commands.h"

/**
 * @brief Execute every external command with or with out argument
 *
 * @param cmd command
 * @param args array containing the command and his arguments
 * @return the return value of the execution of the command : 0 for successed, 1
 * if exec failed, 8 if exec successed but the command or argument is wrong
 */
int extern_command(char *cmd, char **args) {
  int status = 0;
  int return_value = 0;
  // Create child process
  pid_t pid_fork = fork();
  if (pid_fork < 0) {
    write(STDOUT_FILENO, "Error fork\n", 12);
    return 1;
  }

  switch (pid_fork) {
    // If the fork has failed
    case -1:
      write(STDOUT_FILENO, "Error fork\n", 12);
      return 1;
    case 0:
      // Execute the command : if the execution fail, exit with a specific
      // value
      treat_signal(false);
      execvp(cmd, args);
      exit(127);
    default:
      // wait for the end of child process and take his exit value
      waitpid(pid_fork, &status, 0);
      if (WIFSIGNALED(status)) {
        return_value = 255;
      } else if (WIFEXITED(status)) {
        return_value = WEXITSTATUS(status);
      }
      // if the execution of the command has failed
      return return_value;
  }
}

int extern_command_bis(char *cmd, char **args, int fd_write[], int fd_read[]) {
  int status = 0;
  int return_value = 0;
  // Create child process

  pid_t pid_fork = fork();
  if (pid_fork < 0) {
    write(STDOUT_FILENO, "Error fork\n", 12);
    return 1;
  }

  switch (pid_fork) {
    // If the fork has failed
    case -1:
      write(STDOUT_FILENO, "Error fork\n", 12);
      return 1;
    case 0:
      if (fd_write[1] != -1) {
        dup2(fd_write[1], STDOUT_FILENO);
      }
      if (fd_read[0] != -1) {
        dup2(fd_read[0], STDIN_FILENO);
      }
      // Execute the command : if the execution fail, exit with a specific
      // value
      treat_signal(false);
      execvp(cmd, args);
      exit(127);
    default:
      // wait for the end of child process and take his exit value
      waitpid(pid_fork, &status, 0);
      if (WIFSIGNALED(status)) {
        return_value = 255;
      } else if (WIFEXITED(status)) {
        return_value = WEXITSTATUS(status);
      }
      close(fd_write[1]);
      close(fd_read[0]);
      // if the execution of the command has failed
      return return_value;
  }
}