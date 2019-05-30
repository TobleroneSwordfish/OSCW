#include "cat.h"

void main_cat()
{
    int n = 0;
    while(arg(n) != NULL)
    {
        char* path = arg(n);
        int fd = open(path);
        char data[200]; //TODO: implement a malloc syscall with appropriate heap allocated
        read(fd, data, -1);
        int longth = filesize(fd);
        write(STDUSER_FILENO, data, longth);
        write(STDUSER_FILENO, "\n", 1);
        n++;
    }
    exit(0);
}