#include "filetest.h"

void main_filetest()
{
    int fd = open("user/test3");
    char* fdchar;
    itoa(fdchar, fd);
    write(STDOUT_FILENO, "\nfd = ", 6);
    write(STDOUT_FILENO, fdchar, strlen(fdchar));
    write(STDOUT_FILENO, "\n", 1);
    uint8_t data[17] = { 0, 4, 2, 9, 23, 0, 0, 0, 0, 4, 0, 4, 13, 14, 15, 16, 2 };
    write(fd, data, 17);
    uint8_t data2[17];
    read(fd, data2, 17);
    uint8_t data3[17];
    read(fd, data3, -1);
    
    int fd2 = open("user/poke/ahahaa.txt");
    write(fd2, "You found me!", 13);
    char string[14];
    read(fd2, string, -1);
    //write(STDDEBUG_FILENO, string, 13);
    
	int fd3 = open("longboi");
	write(fd3, "This\nis\na\nlooong\nfile\n", strlen("This\nis\na\nlooong\nfile\n"));
    exit(0);
}