#ifndef SIGNAL_H
#define SIGNAL_H

#include <unistd.h>
#include <stdbool.h>
#include <signal.h>
#include <string.h> 

void treat_signal(bool ignore);

#endif