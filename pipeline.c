#include "pipeline.h"

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
      previous_return_value =
          extern_command_bis(cmd[0], cmd, fd_write, fd_read);
    } else {
      previous_return_value = extern_command_bis(cmd[0], cmd, fake, fd_read);
    }
    return previous_return_value;
  }
  if (fd_write[1] != -1) {
    previous_return_value = extern_command_bis(cmd[0], cmd, fd_write, fake);
  }
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
                            char **cmd, int mid) {
  int pos_redirection = 0;
  int length = 0;
  int nbr_redirection = 0;
  int previous_return_value = 0;
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
    if (first_last == 0) {
      previous_return_value = cmd_with_redirection_pipe(
          cmd[0], cmd, length, pos_redirection, fd_write, fd_read);
      return previous_return_value;
    } else if (first_last == 1) {
      previous_return_value = cmd_with_redirection_pipe(
          cmd[0], cmd, length, pos_redirection, fd_write, fd_read);
      return previous_return_value;
    } else {
      previous_return_value = cmd_with_redirection_pipe(
          cmd[0], cmd, length, pos_redirection, fd_write, fd_read);
      return previous_return_value;
    }

  } else {
    previous_return_value = exec_cmd(fd_read, fd_write, cmd);
    return previous_return_value;
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
      exec_first_last_command(0, pipes[0], fake_pipe, tab[0], 0);
  if (previous_return_value != 0) {
    close(pipes[0][1]);
    for (int i = 0; i < length - 1; i++) {
      close(pipes[i][1]);
      close(pipes[i][0]);
    }
    return previous_return_value;
  }
  for (int i = 1; i < length - 1; i++) {
    previous_return_value =
        exec_first_last_command(-1, pipes[i], pipes[i - 1], tab[i], 1);
    if (previous_return_value != 0) {
      close(pipes[i][0]);
      for (int j = i + 1; j < length - 1; j++) {
        close(pipes[j][1]);
        close(pipes[j][0]);
      }
      return previous_return_value;
    }
  }

  previous_return_value = exec_first_last_command(
      1, fake_pipe, pipes[length - 2], tab[length - 1], 0);

  return previous_return_value;
}