#include "P7.h"

void main_P7()
{
    write(STDOUT_FILENO, "start", 5);
    char* x = "test";
    char pid = '0' + fork();
    write(STDOUT_FILENO, &pid, 2);
    write(STDOUT_FILENO, "P7", 2);
    write(STDOUT_FILENO, x, 4);
    exit(0);
}