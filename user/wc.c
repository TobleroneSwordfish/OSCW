#include "wc.h"

void main_wc()
{
	int n = 0;
	while(arg(n) != NULL)
	{
		char* path = arg(n);
		int fd = open(path);
		int longth = filesize(fd);
		char data[200];
		read(fd, data, -1);
		data[longth] = '\0';
		n++;
		char* delimiters = " \t\r\n\v\f"; //posix whitespace characters
		int newlines = 0;
		for (int i = 0; data[i] != '\0'; i++)
		{
			newlines += (data[i] == '\n');
		}
		int words = 0;
		if (strtok(data, delimiters))
		{
			words++;
		}
		while(strtok(NULL, delimiters))
		{
			words++;
		}
		char* ch;
		itoa(ch, newlines);
		write(STDUSER_FILENO, ch, strlen(ch));
		write(STDUSER_FILENO, " ", 1);
		itoa(ch, words);
		write(STDUSER_FILENO, ch, strlen(ch));
		write(STDUSER_FILENO, " ", 1);
		itoa(ch, longth);
		write(STDUSER_FILENO, ch, strlen(ch));
		write(STDUSER_FILENO, " ", 1);
		write(STDUSER_FILENO, path, strlen(path));
		write(STDUSER_FILENO, "\n", 1);
	}
	exit(0);
}