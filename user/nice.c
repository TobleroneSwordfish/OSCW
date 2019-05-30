#include "nice.h"

void main_nice()
{
    int increment = atoi(arg(0));
    pid_t pid = atoi(arg(1));
    nice(increment, pid);
    exit(0);
}