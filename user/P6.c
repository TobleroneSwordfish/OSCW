#include "P6.h"

extern void main_P7();

void main_P6()
{
    write(STDOUT_FILENO, "P6", 2);
    exec(&main_P7);
}