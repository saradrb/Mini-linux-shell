#include "pipeline.h"

#include <limits.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

/**
 * @brief
 *
 * @param x
 * @return int
 */
int close_bis(int x) {
  if (x != -1) {
    if (close(x) == -1) {
      return -2;
    }
  }
  return 0;
}

void test_mini_tab(char **tab) {
  for (int i = 0; tab[i] != NULL; i++) {
    char *tmp = tab[i];
    write(1, tmp, strlen(tmp));
    write(1, "    ", 5);
    char aaa = 'a';
    // write(1, tmp, strlen(tmp));
  }
}

// Alloue de l'espace mémoire pour un char * de la longueur spécifiée
// et initialise tous les éléments à '\0'
char *initialiser_string(int longueur) {
  char *chaine = malloc(longueur * sizeof(char));
  if (chaine == NULL) {
    return NULL;
  }

  // Initialiser tous les éléments à '\0'
  for (int i = 0; i < longueur; i++) {
    chaine[i] = '\0';
  }

  return chaine;
}

void free_triple_tab_test(char ***tableau, int sure) {
  if (sure != -2) return;

  // Pour chaque char ** dans le tableau
  for (int i = 0; tableau[i] != NULL; i++) {
    // Libérer l'espace mémoire alloué pour le char *
    free((*tableau)[i]);
  }
  // Libérer l'espace mémoire alloué pour le char **
  free(*tableau);
  // Mettre le pointeur à NULL pour éviter tout accès futur à cet espace mémoire
  *tableau = NULL;
}

/**
 * @brief
 *
 * @param cmd
 * @param length
 * @return int
 */
int exec_adapted_cmd(char **cmd, int length) {
  test_mini_tab(cmd);
  // compare all possible commands
  if (strcmp(cmd[0], "exit") == 0) {
    return my_exit(cmd + 1, length - 1);
  } else if (strcmp(cmd[0], "cd") == 0) {
    return my_cd(cmd + 1, length - 1);
  } else if (strcmp(cmd[0], "pwd") == 0) {
    return my_pwd(cmd + 1, length - 1);
  } else {
    // execute external command with the new extanded array of args
    int previous_return_value = extern_command(cmd[0], cmd);
    return previous_return_value;
  }
}

/**
 * @brief
 *
 * @param fd_read
 * @param fd_write
 * @param cmd
 * @return int
 */
int exec_cmd(int fd_read[], int fd_write[], char **cmd) {
  int fake[2] = {-1, -1};
  // length de cmd, cmd[0] = commande
  int length = 0;
  for (int i = 0; cmd[i] != NULL; i++) {
    length = length + 1;
  }
  int previous_return_value = -1;

  // regarde si il y a bien un descripteur à lire (si ce n'est pas la première
  // commande)
  if (fd_read[0] != -1) {
    if (fd_write[1] != -1) {
      // // if (dup2(fd_write[1], STDOUT_FILENO) == -1) return -2;
      // // if (dup2(fd_read[0], STDIN_FILENO) == -1) return -2;
      // // // previous_return_value = exec_adapted_cmd(cmd, length);
      // // if (dup2(STDIN_FILENO, fd_read[0]) == -1) return -2;
      // // if (dup2(STDOUT_FILENO, fd_write[1]) == -1) return -2;
      // // if (close(fd_write[1]) == -2) {
      // //   close(fd_read[0]);
      // //   return -2;
      // // }
      previous_return_value =
          extern_command_bis(cmd[0], cmd, fd_write, fd_read);
    } else {
      previous_return_value = extern_command_bis(cmd[0], cmd, fake, fd_read);
      // // // // char *buf = initialiser_string(PIPE_BUF);
      // // // // read(fd_read[0], buf, PIPE_BUF);
      // // // // cmd = realloc(cmd, 4 * sizeof(char *));
      // // // if (dup2(fd_read[0], STDIN_FILENO) == -1) return -2;
      // // // previous_return_value = extern_command(cmd[0], cmd);
      // // // // previous_return_value = exec_adapted_cmd(cmd, length);
      // // // if (dup2(STDIN_FILENO, fd_read[0]) == -1) return -2;
    }
    // // // if (close_bis(fd_read[0]) == -2) return -2;
    return previous_return_value;

    // char *buf = initialiser_string(PIPE_BUF);
    // if (read(fd_read[0], buf, PIPE_BUF) == -1) {
    //   if (close_bis(fd_read[0]) == -2 ||
    //       (fd_write[1] != -1 && close_bis(fd_write[1]) == -2))
    //     return -2;
    //   return -1;
    // }
    // if (close_bis(fd_read[0]) == -2) return -2;
    // test_mini_tab(cmd);
    // cmd = realloc(cmd, sizeof(char *) * (length + 2));
    // if (cmd == NULL) return -2;
    // cmd[length] = buf;
    // cmd[length + 1] = NULL;
    // length = length + 1;
    // test_mini_tab(cmd);
  }
  if (fd_write[1] != -1) {
    previous_return_value = extern_command_bis(cmd[0], cmd, fd_write, fake);
    // // // // if (dup2(fd_write[1], STDOUT_FILENO) == -1) {
    // // // //   return -2;
    // // // // }
    // // // // previous_return_value = exec_adapted_cmd(cmd, length);
    // // // // if (dup2(STDOUT_FILENO, fd_write[1]) == -1) {
    // // // //   return -2;
    // // // // }
    // // // // if (close(fd_write[1]) == -2) return -2;
  } 
  // // // else {
  // // //   // // // test_mini_tab(cmd);
  // // //   // // // previous_return_value = exec_adapted_cmd(cmd, length);
  // // //   previous_return_value = extern_command_bis(cmd[0], cmd, fake, fake);
  // // // }
  return previous_return_value;
}

/**
 * @brief
 *
 * @param first_last
 * @param fd_write
 * @param cmd
 * @return int
 */
int exec_first_last_command(int first_last, int fd_write[], int fd_read[],
                            char **cmd) {
  int fake_pipe[2] = {-1, -1};
  int pos_redirection = 0;
  int length = 0;
  int nbr_redirection = 0;
  int previous_return_value = 0;
  for (int i = 0; cmd[i] != NULL; i++) {
    length = length + 1;
    if (strcmp(cmd[i], "<") == 0 || strcmp(cmd[pos_redirection], ">") == 0 ||
        strcmp(cmd[pos_redirection], ">|") == 0 ||
        strcmp(cmd[pos_redirection], ">>") == 0 ||
        strcmp(cmd[pos_redirection], "2>") == 0 ||
        strcmp(cmd[pos_redirection], "2>>") == 0 ||
        strcmp(cmd[pos_redirection], "2>|") == 0) {
      pos_redirection = i;
      nbr_redirection = nbr_redirection + 1;
    }
  }
  if (nbr_redirection == 1 &&
      ((strcmp(cmd[pos_redirection], "<") == 0 && first_last == 0) ||
       ((strcmp(cmd[pos_redirection], ">") == 0 ||
         strcmp(cmd[pos_redirection], ">|") == 0 ||
         strcmp(cmd[pos_redirection], ">>") == 0 ||
         strcmp(cmd[pos_redirection], "2>") == 0 ||
         strcmp(cmd[pos_redirection], "2>>") == 0 ||
         strcmp(cmd[pos_redirection], "2>|") == 0) &&
        first_last == 1))) {
    if (first_last == 0) {
      if (dup2(fd_write[1], STDOUT_FILENO) == -1) {
        close_bis(fd_write[1]);
        close_bis(fd_write[0]);
        return -2;
      }
      previous_return_value = exec_cmd(fake_pipe, fd_write, cmd);
      // previous_return_value = handle_redirection(cmd[0], cmd);
      if (dup2(STDOUT_FILENO, fd_write[1]) == -1) {
        close_bis(fd_write[1]);
        close_bis(fd_write[0]);
        return -2;
      }

      return previous_return_value;
    } else {
      previous_return_value = exec_cmd(fd_read, fake_pipe, cmd);
      return previous_return_value;
      // previous_return_value = handle_redirection(cmd[length - 1], cmd);
    }
  } else {
    if (first_last == 0) {
      previous_return_value = exec_cmd(fake_pipe, fd_write, cmd);
      return previous_return_value;
    } else {
      previous_return_value = exec_cmd(fd_read, fake_pipe, cmd);
      return previous_return_value;
    }
  }
}

int lenght_of_pipelines(char ***tab) {
  int length = 0;
  for (int i = 0; tab[i] != NULL; i++) {
    length = length + 1;
  }
  return length;
}

/**
 * @brief
 *
 * @param tab
 * @param length
 * @return int
 */
int exec_pipeline(char ***tab) {
  int length = lenght_of_pipelines(tab);
  // close_bis(pipes[length - 2][0]);
  int fake_pipe[2] = {-1, -1};
  // Créez le tableau de pipes
  int pipes[length - 1][2];

  // Initialisez chaque pipe dans le tableau
  for (int i = 0; i < length - 1; i++) {
    if (pipe(pipes[i]) == -1) return -2;
  }

  int previous_return_value =
      exec_first_last_command(0, pipes[0], fake_pipe, tab[0]);
  if (previous_return_value != 0) {
    close(pipes[0][1]);
    for (int i = 0; i < length - 1; i++) {
      close(pipes[i][1]);
      close(pipes[i][0]);
    }
    return previous_return_value;
  }
  for (int i = 1; i < length - 1; i++) {
    previous_return_value = exec_cmd(pipes[i - 1], pipes[i], tab[i]);
    if (previous_return_value != 0) {
      close(pipes[i][0]);
      for (int j = i + 1; j < length - 1; j++) {
        close(pipes[j][1]);
        close(pipes[j][0]);
      }
      return previous_return_value;
    }
  }

  previous_return_value =
      exec_first_last_command(1, fake_pipe, pipes[length - 2], tab[length - 1]);

  return previous_return_value;
}

/*char **copy_part_of_cmd(char **cmd, int start, int finish) {
  char **res = malloc((finish + 2 - start) * sizeof(char *));
  if (res == NULL) {
    perror("malloc");
  }

  for (int i = start; i <= finish; i++) {
    char *test = cmd[i];
    res[i - start] = cmd[i];
  }
  res[finish + 1 - start] = NULL;
  if (res[finish + 1 - start] == NULL) {
    int a = 5;
  }
  return res;
}

void test_mni_tab(char **tmp) {
  for (int j = 0; tmp[j] != NULL; j++) {
    char *res = tmp[j];
    char a = 'r';
  }
}

void test_tab(char ***tmp) {
  for (int i = 0; tmp[i] != NULL; i++) {
    char **ttmp = tmp[i];
    for (int j = 0; ttmp[j] != NULL; j++) {
      char *res = ttmp[j];
      char a = 'r';
    }
  }
}

char ***split_cmd_to_pipeline(char **cmd, int nbr_of_pipes) {
  char ***res = malloc((nbr_of_pipes + 2) * sizeof(char **));
  int i = 0;
  int prev = 0;
  int j = 0;
  while (cmd[j] != NULL) {
    char *tmp = cmd[j];
    if (strcmp(cmd[j], "|") == 0) {
      // res[i] = realloc(cmd[prev], (j + 1 - prev) * sizeof(char *));
      res[i] = copy_part_of_cmd(cmd, prev, j - 1);
      test_mni_tab(res[i]);
      i = i + 1;
      prev = j + 1;
      j = j + 1;
    } else {
      j = j + 1;
    }
  }
  res[i] = copy_part_of_cmd(cmd, prev, j - 1);
  test_mni_tab(res[i]);
  res[i + 1] = NULL;
  return res;
}

int nbr_pipe(char **tab) {
  int i = 0;
  int len = 0;
  while (tab[i]) {
    if (strcmp(tab[i], "|") == 0) {
      len++;
    }
    i++;
  }
  return len;
}*/