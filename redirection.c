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
  for (int i = 0; i < size; i++) {
    char* string = args_extanded[i];
    if (strcmp(string, ">") == 0 || strcmp(string, "<") == 0 ||
        strcmp(string, ">>") == 0 || strcmp(string, ">|") == 0 ||
        strcmp(string, "2>") == 0 || strcmp(string, "2>>") == 0 ||
        strcmp(string, "2>|") == 0 || strcmp(string, "2>") == 0) {
      if (i < size - 1 && i > 0)
        return i;  // redirection symbol position
      else
        return -1;  // means invalid redirection
    }
  }
  return -2;  // means no redirection
}

/**
 * @brief handle a command redirection
 *
 * @param redirection string that contains the redirection sign
 * @param filename the name of the destination file
 * @return return an array that contains the default descriptors of stdin dtdout
 * and stderr
 *
 */
int* handle_redirection(char* redirection, char* filename) {
  int fd = 0;
  static int fd_standard[3];
  fd_standard[0] = dup(STDIN_FILENO);
  fd_standard[1] = dup(STDOUT_FILENO);
  fd_standard[2] = dup(STDERR_FILENO);
  int result;
  if (strcmp(redirection, "<") ==
      0) {  // redirecting stdin of the command to the file filename

    fd = open(filename, O_RDONLY);
    if (fd == -1) {
      perror("opening file error");
      return NULL;
    }

    result = dup2(fd, STDIN_FILENO);
    if (result == -1) {
      perror("error redirecting stdin");
      return NULL;
    }
    close(fd);
  } else {
    if (strcmp(redirection, ">") == 0) {
      fd = open(filename, O_WRONLY | O_CREAT | O_EXCL, 0644);
      if (fd == -1) {
        perror("opening file error");
        return NULL;
      }

      result = dup2(fd, STDOUT_FILENO);
      if (result == -1) {
        perror("error redirecting stdout");
        return NULL;
      }
      close(fd);

    } else {
      if (strcmp(redirection, ">|") == 0) {
        fd = open(filename, O_WRONLY | O_CREAT | O_TRUNC, 0644);
        if (fd == -1) {
          perror("opening file error");
          return NULL;
        }

        result = dup2(fd, STDOUT_FILENO);
        if (result == -1) {
          perror("error redirecting stdout");
          return NULL;
        }
        close(fd);

      } else {
        if (strcmp(redirection, ">>") == 0) {
          fd = open(filename, O_WRONLY | O_CREAT | O_APPEND, 0644);
          if (fd == -1) {
            perror("opening file error");
            return NULL;
          }

          result = dup2(fd, STDOUT_FILENO);
          if (result == -1) {
            perror("error redirecting stdout");
            return NULL;
          }
          close(fd);

        } else {
          if (strcmp(redirection, "2>") == 0 ||
              strcmp(redirection, "2>|") == 0 ||
              strcmp(redirection, "2>>") == 0) {
            if (strcmp(redirection, "2>") == 0) {
              fd = open(filename, O_WRONLY | O_CREAT | O_EXCL, 0644);

            } else {
              if (strcmp(redirection, "2>|") == 0) {
                fd = open(filename, O_WRONLY | O_CREAT | O_TRUNC, 0644);

              } else {
                fd = open(filename, O_WRONLY | O_CREAT | O_APPEND, 0644);
              }
            }

            if (fd == -1) {
              perror("opening file error");
              return NULL;
            }

            result = dup2(fd, STDERR_FILENO);
            if (result == -1) {
              perror("error redirecting stderr");
              return NULL;
            }
            close(fd);
          } else
            return NULL;
        }
      }
    }
  }
  return fd_standard;
}



// go back to standard descriptors for stdin stdout and stderr, after a
// redirection
void go_back_to_standard(int* fd_std) {
  dup2(fd_std[0], STDIN_FILENO);
  close(fd_std[0]);

  dup2(fd_std[1], STDOUT_FILENO);
  close(fd_std[1]);

  dup2(fd_std[3], STDERR_FILENO);
  close(fd_std[2]);
}

