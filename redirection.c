#include "redirection.h"

/**
 * @brief check if the arg list contains a valid redirection (redirection symbol
 * and a filename destination)
 *
 * @param args_extanded argument array
 * @param size size of the array , number of args
 * @return return the position of the redirection symbol in array if finded
 * returns -1 if the redirection is invalid
 * returns -2 if there is no redirection
 */
int contains_valid_redirection(char** args_extanded, int size) {
  for (int i = size - 1; i > 0; i--) {
    char* string = args_extanded[i];
    if (strcmp(string, ">") == 0 || strcmp(string, "<") == 0 ||
        strcmp(string, ">>") == 0 || strcmp(string, ">|") == 0 ||
        strcmp(string, "2>") == 0 || strcmp(string, "2>>") == 0 ||
        strcmp(string, "2>|") == 0 || strcmp(string, "2>") == 0) {
      // here args_extanded[i+1] is the redirection field and it can't be a pipe
      if (i < size - 1 && !prefix("|", args_extanded[i + 1], NULL)) {
        return i;
      }  // redirection symbol position
      else {
        return 0;  // invalid redirection
      }
    }
  }

  return -1;  // means no redirection
}

/**
 * @brief handle a command redirection
 *
 * @param redirection string that contains the redirection sign
 * @param filename the name of the destination file
 * @return return 0 if the redirection has been done or 1
 */
int handle_redirection(char* redirection, char* filename) {
  int fd = 0;
  int result;
  if (strcmp(redirection, "<") ==
      0) {  // redirecting stdin of the command to the file filename

    fd = open(filename, O_RDONLY);
    if (fd == -1) {
      perror("opening file error");
      return 1;
    }

    result = dup2(fd, STDIN_FILENO);
    if (result == -1) {
      perror("error redirecting stdin");
      return 1;
    }
    close(fd);
  } else {
    if (strcmp(redirection, ">") == 0) {
      fd = open(filename, O_WRONLY | O_CREAT | O_EXCL, 0666);
      if (fd == -1) {
        perror("opening file error");
        return 1;
      }

      result = dup2(fd, STDOUT_FILENO);
      if (result == -1) {
        perror("error redirecting stdout");
        return 1;
      }
      close(fd);

    } else {
      if (strcmp(redirection, ">|") == 0) {
        fd = open(filename, O_WRONLY | O_CREAT | O_TRUNC, 0666);
        if (fd == -1) {
          perror("opening file error");
          return 1;
        }

        result = dup2(fd, STDOUT_FILENO);
        if (result == -1) {
          perror("error redirecting stdout");
          return 1;
        }
        close(fd);

      } else {
        if (strcmp(redirection, ">>") == 0) {
          fd = open(filename, O_WRONLY | O_CREAT | O_APPEND, 0644);
          if (fd == -1) {
            perror("opening file error");
            return 1;
          }

          result = dup2(fd, STDOUT_FILENO);
          if (result == -1) {
            perror("error redirecting stdout");
            return 1;
          }
          close(fd);

        } else {
          if (strcmp(redirection, "2>") == 0 ||
              strcmp(redirection, "2>|") == 0 ||
              strcmp(redirection, "2>>") == 0) {
            if (strcmp(redirection, "2>") == 0) {
              fd = open(filename, O_WRONLY | O_CREAT | O_EXCL, 0664);

            } else {
              if (strcmp(redirection, "2>|") == 0) {
                fd = open(filename, O_WRONLY | O_CREAT | O_TRUNC, 0644);

              } else {
                fd = open(filename, O_WRONLY | O_CREAT | O_APPEND, 0644);
              }
            }

            if (fd == -1) {
              perror("opening file error");
              return 1;
            }

            result = dup2(fd, STDERR_FILENO);
            if (result == -1) {
              perror("error redirecting stderr");
              return 1;
            }
            close(fd);
          } else
            return 1;
        }
      }
    }
  }
  return 0;
}

// go back to standard descriptors for stdin stdout and stderr, after a
// redirection
void go_back_to_standard(int* fd_std) {
  dup2(fd_std[0], STDIN_FILENO);
  close(fd_std[0]);

  dup2(fd_std[1], STDOUT_FILENO);
  close(fd_std[1]);

  dup2(fd_std[2], STDERR_FILENO);
  close(fd_std[2]);
}

// execute cmd with a redirection
int cmd_with_redirection(char* cmd, char** args, int length,
                         int pos_redirection) {
  static int fd_standard[3];
  fd_standard[0] = dup(STDIN_FILENO);
  fd_standard[1] = dup(STDOUT_FILENO);
  fd_standard[2] = dup(STDERR_FILENO);
  int return_value = 0;
  char* sv = args[pos_redirection];
  if (pos_redirection == 0) {
    return 2;
  }
handle_rd:
  length = length - 2;
  return_value = handle_redirection(args[length], args[length + 1]);
  if (return_value == 0) {
    if (contains_valid_redirection(args, length) > 0) {
      goto handle_rd;
    }
    sv = args[length];    // save the adress of the argument
    args[length] = NULL;  // put the pointer to argumnt = null so that the
                          // commands will stop at null
    if (strcmp(cmd, "exit") == 0) {
      return_value = my_exit(args + 1, length - 1);
    } else {
      if (strcmp(cmd, "cd") == 0) {
        return_value = my_cd(args + 1, length - 1);
      } else {
        if (strcmp(cmd, "pwd") == 0) {
          return_value = my_pwd(args + 1, length - 1);
        } else {
          // execute external command with the new extanded array of args
          return_value = extern_command(cmd, args);
        }
      }
    }
  }
  args[length] = sv;
  go_back_to_standard(fd_standard);
  return (return_value);
}

int cmd_with_redirection_pipe(char* cmd, char** args, int length,
                              int pos_redirection, int fd_write[],
                              int fd_read[]) {
  static int fd_standard[3];
  fd_standard[0] = dup(STDIN_FILENO);
  fd_standard[1] = dup(STDOUT_FILENO);
  fd_standard[2] = dup(STDERR_FILENO);
  int return_value = 0;
handle_rd:
  length = length - 2;
  return_value = handle_redirection(args[length], args[length + 1]);
  args[length] = NULL;

  if (return_value == 0) {
    if (contains_valid_redirection(args, length) > 0) {
      goto handle_rd;
    }
    // execute external command with the new extanded array of args
    return_value = extern_command_bis(cmd, args, fd_write, fd_read);
  }
  go_back_to_standard(fd_standard);
  return (return_value);
}
