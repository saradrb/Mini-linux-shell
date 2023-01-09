#ifndef PIPELINE_H
#define PIPELINE_H

#include <limits.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "external_commands.h"
// #include "internal_commands.h"
#include "redirection.h"

int close_bis(int x);
void test_mini_tab(char **tab);
char *initialiser_string(int longueur);
void free_triple_tab_test(char ***tableau, int sure);
int exec_adapted_cmd(char **cmd, int length, int fd_write[], int fd_read[]);
int exec_cmd(int fd_read[], int fd_write[], char **cmd);
int exec_first_last_command(int first_last, int fd_write[], int fd_read[],
                            char **cmd, int mid);
int lenght_of_pipelines(char ***tab);
int exec_pipeline(char ***tab);

#endif