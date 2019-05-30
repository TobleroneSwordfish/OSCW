#include "runprograms.h"

extern void main_P3();
extern void main_P4();    
extern void main_P5();

void main_runprograms()
{
    if(!fork())
    {
        exec(&main_P3);
    }
    if(!fork())
    {
        exec(&main_P4);
    }
    if(!fork())
    {
        exec(&main_P5);
    }
    exit(0);
}