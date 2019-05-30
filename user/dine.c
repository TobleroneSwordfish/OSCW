#include "dine.h"

extern void main_phil();

void main_dine()
{
    char* ch = arg(0);
    int phil_no = atoi(ch);
	pid_t pids[100]; //can't use malloc in user programs currently
    for (int i = 0; i < phil_no; i++)
    {
		pid_t pid = fork();
        if (pid == 0)
        {
            exec(&main_phil);
        }
		else
		{
			pids[i] = pid;
		}
    }
	for (int i = 0; i < phil_no; i++)
	{
		write(STDOUT_FILENO, "\nWaiting for phil ", 18);
        char ch[4];
        itoa(ch, i);
		write(STDOUT_FILENO, ch, strlen(ch));
        write(STDOUT_FILENO, "\n", 1);
		wait(pids[i]);
	}
    write(STDOUT_FILENO, "All philosphers returned\n", 25);
    exit(EXIT_SUCCESS);
}