#include "wildcard.h"

extern char current_rep[];

// parse a path with a delimiter
char **parse_path(char *path, int *length, char *delimiters) {
  char **list_arg = NULL;
  char *arg = NULL;
  *length = 0;
  int nb_spaces = 0;
  if (path[0] == '/') {
    nb_spaces = 1;
    list_arg = realloc(list_arg, sizeof(char *) * nb_spaces);
    char *myslash = "/";
    if (list_arg == NULL) exit(-1);  // memory allocation failed
    list_arg[nb_spaces - 1] = myslash;
  }
  arg = strtok(path, delimiters);
  while (arg && (nb_spaces < MAX_ARGS_NUMBER)) {
    nb_spaces++;
    list_arg = realloc(list_arg, sizeof(char *) * nb_spaces);
    if (list_arg == NULL) exit(-1);  // memory allocation failed
    list_arg[nb_spaces - 1] = arg;
    arg = strtok(NULL, delimiters);
  }
  list_arg = realloc(list_arg, sizeof(char *) * (nb_spaces + 1));
  list_arg[nb_spaces] = NULL;

  *length = nb_spaces;
  return list_arg;
}

int prefix(char *string, char *str, char *new_str) {
  if (strncmp(string, str, strlen(string)) == 0) {
    if (strlen(str) == strlen(string)) {
      (new_str) = NULL;
    } else {
      if (new_str != NULL) {
        strcpy(new_str, str + strlen(string));
        strcat(new_str, "\0");
      }
    }
    return 1;
  } else {
    return 0;
  }
}

int suffix(char *string, char *str, char *new_str) {
  char *ending = str + strlen(str) - strlen(string);
  // printf("ending: %s\n",ending);
  if (strcmp(string, ending) == 0) {
    if (strlen(str) == strlen(string)) {
      new_str = NULL;
    } else {
      if (new_str != NULL) {
        strncpy(new_str, str, strlen(str) - strlen(ending));
        strcat(new_str, "\0");
      }
    }
    return 1;
  } else {
    return 0;
  }
}

// function that return in the array "options" all the path options after
// expansion of the * wildcard
int expand_star(char **path, int length, char *expanded_path, char **options,
                int *nb_options) {
  struct dirent *entry = NULL;
  DIR *current_dir = NULL;
  struct stat st;
  char *rest_path = malloc(sizeof(char) * PATH_MAX);
  memset(rest_path, 0, sizeof(char) * PATH_MAX);
  char *fullpath = malloc(sizeof(char) * PATH_MAX);
  memset(fullpath, 0, sizeof(char) * PATH_MAX);
  int i = 0;

expand:
  // printf("current dir is %s\n",expanded_path);
  if (strcmp(expanded_path, "") == 0) {
    current_dir = opendir(current_rep);
  } else {
    current_dir = opendir(expanded_path);
  }
  if (current_dir == NULL) {
    free(rest_path);
    free(fullpath);
    return (1);
  }

  if (length == 1) {  // case 1: path has one element
    // printf("path i is %s\n",path[length]);
    while ((entry = readdir(current_dir))) {
      if (strcmp(expanded_path, "") != 0) {
        if (suffix("/", expanded_path, NULL))
          sprintf(fullpath, "%s%s", expanded_path, entry->d_name);
        else
          sprintf(fullpath, "%s/%s", expanded_path, entry->d_name);
      } else {
        sprintf(fullpath, "%s", entry->d_name);
      }

      if (stat(fullpath, &st) == -1) {
        perror("stat failed ");
        free(rest_path);
        free(fullpath);
        return (1);
      }

      if (prefix("*", path[i], rest_path)) {
        if (!prefix(".", entry->d_name, NULL) &&
            suffix(rest_path, entry->d_name, NULL)) {
          options[*nb_options] = malloc(sizeof(char) * PATH_MAX);
          sprintf(options[*nb_options], "%s", fullpath);
          (*nb_options)++;
        }
      } else {  // path doesn't contain a *

        if ((strcmp(path[i], entry->d_name) == 0) &&
            !suffix(".", entry->d_name, NULL)) {
          options[*nb_options] = malloc(sizeof(char) * PATH_MAX);
          sprintf(options[*nb_options], "%s", fullpath);
          (*nb_options)++;
        }
      }
    }

  } else {                                  // case 2: diractory
    if (prefix("*", path[i], rest_path)) {  // dir contains a *
      length--;
      path = &path[i + 1];
      while ((entry = readdir(current_dir))) {
        if (strcmp(expanded_path, "") != 0) {
          if (suffix("/", expanded_path, NULL))
            sprintf(fullpath, "%s%s", expanded_path, entry->d_name);
          else
            sprintf(fullpath, "%s/%s", expanded_path, entry->d_name);
        } else {
          sprintf(fullpath, "%s", entry->d_name);
        }

        if (stat(fullpath, &st) == -1) {
          perror("stat failed ");
          free(rest_path);
          free(fullpath);
          return (1);
        }

        if (S_ISDIR(st.st_mode) && suffix(rest_path, entry->d_name, NULL) &&
            !prefix(".", entry->d_name, NULL)) {
          expand_star(path, length, fullpath, options, nb_options);
        }
      }

    } else {  // dir doesn't contain a *

      if (strcmp(expanded_path, "") != 0 && strcmp(expanded_path, "/") != 0) {
        strncat(expanded_path, "/", 2);
      }
      strncat(expanded_path, path[i], strlen(path[i]));
      closedir(current_dir);
      i++;
      length--;
      goto expand;
    }
  }
  closedir(current_dir);
  free(rest_path);
  free(fullpath);
  return (0);
}

// function that checks if an array of char* contains an element that contains a
// wildcard
int contains_wildcard(char **tab_argument, int length, const char *wildcard) {
  for (int i = 0; i < length; i++) {
    if (strstr(tab_argument[i], wildcard)) return i;
  }
  return (-1);
}

// expand the ** joker, it take the path after the ** and search for it in the
// current repo tree

char **expand_double_star(char **path, int length, char *current_repo,
                          char **path_options, int *nb_op) {
  DIR *mydir;
  struct dirent *entry;
  char *fullpath = malloc(sizeof(char) * PATH_MAX);
  memset(fullpath, 0,
         sizeof(char) * PATH_MAX);  // full path with the working dir included
  char result[PATH_MAX] = {'\0'};
  // strcpy(result,current_repo);

  // create fullpath
  sprintf(fullpath, "%s/%s", current_rep, current_repo);

  mydir = opendir(fullpath);
  free(fullpath);
  if (mydir == NULL) {
    perror("opendir failed ");
    return (NULL);
  }

  // if we are looking for a specific dir or file we search with expand_star
  if (length > 0) {
    int size = 0;
    char **options = malloc(sizeof(char *) * PATH_MAX);
    expand_star(path, length, current_repo, options, &size);
    path_options = concat_tab(path_options, nb_op, options, size);
    free(options);
  }

  while ((entry = readdir(mydir))) {
    if (!prefix(".", entry->d_name, NULL) && (entry->d_type == DT_DIR)) {
      snprintf(result, PATH_MAX, "%s%s/", current_repo, entry->d_name);

      // if length==0 means we have **/ so we look for all the subdirectories in
      // the tree
      if (length == 0) {
        // printf("*nb op%d\n",*nb_op);
        int len = 0;
        char **ptr = parse_path(result, &len, " ");
        path_options = concat_elem(path_options, nb_op, result);
        free(ptr);
      }

      path_options =
          expand_double_star(path, length, result, path_options, nb_op);
    }
  }

  closedir(mydir);
  return (path_options);
}