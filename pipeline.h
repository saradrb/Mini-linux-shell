#ifndef PIPELINE_H
#define PIPELINE_H

#include <limits.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "external_commands.h"
#include "redirection.h"

// int exec_cmd(int fd_read[], int fd_write[], char **cmd);
int exec_first_last_command(int first_last, int fd_write[], int fd_read[],
                            char **cmd, int mid);
int length_of_pipelines(char ***tab);
int exec_pipeline(char ***tab);

#endif