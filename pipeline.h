#ifndef PIPELINE_H
#define PIPELINE_H

#include "external_commands.h"
#include "internal_commands.h"
int close_bis(int x);
void test_mini_tab(char **tab);
char *initialiser_string(int longueur);
void free_triple_tab(char ***tableau, int sure);
int exec_adapted_cmd(char **cmd, int length);
int exec_cmd(int fd_read[], int fd_write[], char **cmd);
int exec_first_last_command(int first_last, int fd_write[], int fd_read[],
                            char **cmd);
int lenght_of_pipelines(char ***tab);
int exec_pipeline(char ***tab);

#endif