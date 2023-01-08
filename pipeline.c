#include <limits.h>
#include <stddef.h>
#include <stdlib.h>
#include <unistd.h>

/**
 * @brief
 *
 * @param tab
 * @param length
 * @return int
 */
int exec_pipeline(char ***tab, int length) {
  // Créez le tableau de pipes
  int pipes[length - 1][2];

  // Initialisez chaque pipe dans le tableau
  for (int i = 0; i < length; i++) {
    if (pipe(pipes[i]) == -1) return -2;
  }
  // appeller la methode qui check si il y a une redirection dans la première
  // commande de pipe

  for (int i = 1; i < length - 1; i++) {
    int result_exec = 0;
  }

  // appeller la méthode qui check si il y a une redirection dans la dernière
  // commande de pipe
}

int exec_first_last_command(int first_last, int fd_write[], char **cmd) {
  int pos_redirection = 0;
  int length = 0;
  int nbr_redirection = 0;
  int previous_return_value = 0;
  for (int i = 0; cmd[i] != NULL; i++) {
    length = length + 1;
    if (strcmp(cmd[i], "<") || strcmp(cmd[i], "<") || strcmp(cmd[i], "<") || strcmp(cmd[i], "<")) {
      pos_redirection = i;
      nbr_redirection = nbr_redirection + 1;
    }
  }
  if (nbr_redirection == 1 && ((strcmp(cmd[pos_redirection], "<") && first_last == 0) || (strcmp(cmd[pos_redirection], "<") && first_last != 0))) {
    if (dup2(fd_write[1], STDOUT_FILENO) == -1) {
      close_bis(fd_write[1]);
      close_bis(fd_write[0]);
      return -2;
    }
    previous_return_value = handle_redirection(cmd[0], cmd);
    if (dup2(STDOUT_FILENO, fd_write[1]) == -1) {
      close_bis(fd_write[1]);
      close_bis(fd_write[0]);
      return -2;
    }
  } else {
    exec_adapted_cmd(cmd, length);
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
  if (close_bis(fd_read[1]) == -2 || close_bis(fd_write[0]) == -2) return -2;
  // length de cmd cmd[0] = commande
  int length = 0;
  for (int i = 0; cmd[i] != NULL; i++) {
    length = length + 1;
  }
  // regarde si il y a bien un descripteur à lire (si ce n'est pas la première
  // commande)
  if (fd_read != -1) {
    char *buf[PIPE_BUF] = initialiser_string(PIPE_BUF);
    if (read(fd_read[0], buf, PIPE_BUF) == -1) {
      if (close_bis(fd_read[0]) == -2 || close_bis(fd_write[1]) == -2)
        return -2;
      return -1;
    }
    if (close_bis(fd_read[0]) == -2) return -2;
    cmd = realloc(cmd, sizeof(char *) * (length + 1));
    if (cmd == NULL) return -2;
    cmd[length] = buf;
    length = length + 1;
  }
  int previous_return_value = 0;
  if (fd_write != -1) {
    if (dup2(fd_write[1], STDOUT_FILENO) == -1) {
      close_bis(fd_write[1]);
      return -2;
    }
    previous_return_value = exec_adapted_cmd(cmd, length);
    if (close_bis(fd_write[1]) == -2) return -2;
    if (dup2(STDOUT_FILENO, fd_write[1]) == -1) {
      return -2;
    }
  } else {
    previous_return_value = exec_adapted_cmd(cmd, length);
  }

  return previous_return_value;
}

/**
 * @brief
 *
 * @param cmd
 * @param length
 * @return int
 */
int exec_adapted_cmd(char **cmd, int length) {
  // compare all possible commands
  if (strcmp(cmd, "exit") == 0) {
    return my_exit(cmd + 1, length);
  } else if (strcmp(cmd, "cd") == 0) {
    return my_cd(cmd + 1, length);
  } else if (strcmp(cmd, "pwd") == 0) {
    return my_pwd(cmd + 1, length);
  } else {
    // execute external command with the new extanded array of args
    return extern_command(cmd[0], cmd);
  }
}

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

void free_triple_tab(char ***tableau, int sure) {
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
 * @brief TODO:
 * 
 * 
 */