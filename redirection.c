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

    if (size-2 > 0){
      char* string = args_extanded[size-2];
      if (strcmp(string, ">") == 0 || strcmp(string, "<") == 0 ||
          strcmp(string, ">>") == 0 || strcmp(string, ">|") == 0 ||
          strcmp(string, "2>") == 0 || strcmp(string, "2>>") == 0 ||
          strcmp(string, "2>|") == 0 || strcmp(string, "2>") == 0) {
          return (size-2);
        }  // redirection symbol position
    }
  
  return -1;  // means no redirection
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
int handle_redirection(char* redirection, char* filename) {
  int fd = 0;
  int result;
  if (strcmp(redirection, "<") ==0) {  // redirecting stdin of the command to the file filename

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

