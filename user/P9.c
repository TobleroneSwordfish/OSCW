#include "P9.h"

extern void main_P10();

void main_P9()
{
    pid_t pid = fork();
    char ret = '0' + pid;
    write(STDOUT_FILENO, "\nFork returned ", 15);
    write(STDOUT_FILENO, &ret, 1);
    if (pid == 0)
    {
        exec(&main_P10);
    }
    write(STDOUT_FILENO, "\ncalling wait", 13);
    wait(pid);
    int result = inspect(pid);
    char ch = '0' + result;
    write(STDOUT_FILENO, "\ninspect returned", 17);
    write(STDOUT_FILENO, &ch, 1);
    exit(result);
}