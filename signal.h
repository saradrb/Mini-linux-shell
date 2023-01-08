#ifndef SIGNAL_H
#define SIGNAL_H

#include <signal.h>
#include <stdbool.h>
#include <string.h>
#include <unistd.h>

void treat_signal(bool ignore);

#endif