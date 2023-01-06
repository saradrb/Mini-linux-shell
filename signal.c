#include "signal.h"
#include <stdio.h>

void treat_signal(bool ignore) {
    struct sigaction action;
    memset(&action, 0, sizeof(struct sigaction));

    if (ignore) action.sa_handler = SIG_IGN;
    else action.sa_handler = SIG_DFL;

    sigaction(SIGINT, &action, NULL);
    sigaction(SIGTERM, &action, NULL);
}