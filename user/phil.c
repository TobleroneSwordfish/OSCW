#include "phil.h"

extern int is_prime(uint32_t x);

unsigned short lfsr = 0xACE1u;
unsigned bit;

unsigned rand()
{
  bit  = ((lfsr >> 0) ^ (lfsr >> 2) ^ (lfsr >> 3) ^ (lfsr >> 5) ) & 1;
  return lfsr =  (lfsr >> 1) | (bit << 15);
}

void main_phil()
{
    write(STDOUT_FILENO, "Start", 5);
    int i = 0; //which philosopher are we
    int r = claim(0, 1 << i);
    char newlin = '\n';
    while(r != 0)
    {
        i++;
		r = claim(0, 1 << i);
    }
    char ch[4];
    itoa(ch, i);
    write(STDOUT_FILENO, "Phil: ", 6);
    write(STDOUT_FILENO, ch, strlen(ch));
    write(STDOUT_FILENO, &newlin, 1);
    uint32_t go = (1 << i) | (1 << ((i + 1) % 16));
    while (claim(50, go) != 0)
    {
        write(STDOUT_FILENO, "Phil ", 5);
        write(STDOUT_FILENO, ch, strlen(ch));
        write(STDOUT_FILENO, " blocked\n", 9);
        uint32_t lo = 1 <<  8;
        uint32_t hi = 1 << 16;

        for( uint32_t x = lo; x < hi; x++ )
        {
            int r = is_prime( x ); 
        }
    }
    write(STDOUT_FILENO, "Phil ", 5);
    write(STDOUT_FILENO, ch, strlen(ch));
    write(STDOUT_FILENO, " dining\n", 8);
    int ra = rand() * 10000;
    while(ra > 0)
    {
        ra--;
    }
    write(STDOUT_FILENO, "Phil ", 5);
    write(STDOUT_FILENO, ch, strlen(ch));
    write(STDOUT_FILENO, " dined\n", 7);
    
    release(50, go);
    
    exit(0);
}