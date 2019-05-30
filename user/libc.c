/* Copyright (C) 2017 Daniel Page <csdsp@bristol.ac.uk>
 *
 * Use of this source code is restricted per the CC BY-NC-ND license, a copy of 
 * which can be found via http://creativecommons.org (and should be included as 
 * LICENSE.txt within the associated archive or repository).
 */

#include "libc.h"

int  atoi( char* x        ) {
  char* p = x; bool s = false; int r = 0;

  if     ( *p == '-' ) {
    s =  true; p++;
  }
  else if( *p == '+' ) {
    s = false; p++;
  }

  for( int i = 0; *p != '\x00'; i++, p++ ) {
    r = s ? ( r * 10 ) - ( *p - '0' ) :
            ( r * 10 ) + ( *p - '0' ) ;
  }

  return r;
}

void itoa( char* r, int x ) {
  char* p = r; int t, n;

  if( x < 0 ) {
     p++; t = -x; n = t;
  }
  else {
          t = +x; n = t;
  }

  do {
     p++;                    n /= 10;
  } while( n );

    *p-- = '\x00';

  do {
    *p-- = '0' + ( t % 10 ); t /= 10;
  } while( t );

  if( x < 0 ) {
    *p-- = '-';
  }

  return;
}

void yield() {
  asm volatile( "svc %0     \n" // make system call SYS_YIELD
              :
              : "I" (SYS_YIELD)
              : );

  return;
}

int write( int fd, const void* x, size_t n ) {
  int r;

  asm volatile( "mov r0, %2 \n" // assign r0 = fd
                "mov r1, %3 \n" // assign r1 =  x
                "mov r2, %4 \n" // assign r2 =  n
                "svc %1     \n" // make system call SYS_WRITE
                "mov %0, r0 \n" // assign r  = r0
              : "=r" (r) 
              : "I" (SYS_WRITE), "r" (fd), "r" (x), "r" (n)
              : "r0", "r1", "r2" );

  return r;
}

int  read( int fd,       void* x, size_t n ) {
  int r;

  asm volatile( "mov r0, %2 \n" // assign r0 = fd
                "mov r1, %3 \n" // assign r1 =  x
                "mov r2, %4 \n" // assign r2 =  n
                "svc %1     \n" // make system call SYS_READ
                "mov %0, r0 \n" // assign r  = r0
              : "=r" (r) 
              : "I" (SYS_READ),  "r" (fd), "r" (x), "r" (n) 
              : "r0", "r1", "r2" );

  return r;
}

int  fork() {
  int r;

  asm volatile( "svc %1     \n" // make system call SYS_FORK
                "mov %0, r0 \n" // assign r  = r0 
              : "=r" (r) 
              : "I" (SYS_FORK)
              : "r0" );

  return r;
}

void exit( int x ) {
  asm volatile( "mov r0, %1 \n" // assign r0 =  x
                "svc %0     \n" // make system call SYS_EXIT
              :
              : "I" (SYS_EXIT), "r" (x)
              : "r0" );

  return;
}

void execa( const void* x , char** args, int n ) {
//   char** aargs[10];
//   for (int i = 0; i < n; i++)
//   {
// 	  aargs[i] = strdup(args[i]);
//   }
  asm volatile( "mov r0, %1 \n" // assign r0 = x
                "mov r1, %2 \n" // assing r1 = args pointer
			    "mov r2, %3 \n" // assign r2 = n
                "svc %0     \n" // make system call SYS_EXEC
              :
              : "I" (SYS_EXEC), "r" (x), "r" (args), "r" (n)
              : "r0", "r1", "r2" );
	
  return;
}

void exec( const void* x )
{
    execa(x, NULL, 0);
}

int  kill( int pid, int x ) {
  int r;

  asm volatile( "mov r0, %2 \n" // assign r0 =  pid
                "mov r1, %3 \n" // assign r1 =    x
                "svc %1     \n" // make system call SYS_KILL
                "mov %0, r0 \n" // assign r0 =    r
              : "=r" (r) 
              : "I" (SYS_KILL), "r" (pid), "r" (x)
              : "r0", "r1" );

  return r;
}

void nice(int increment, pid_t pid)
{
    asm volatile( "mov r0, %1 \n" // assign r0 =  pid
                  "mov r1, %2 \n" // assign r1 =  increment
                  "svc %0     \n" // make system call SYS_NICE
                  :
                  : "I" (SYS_NICE), "r" (pid), "r" (increment)
                  : "r0", "r1" );
}

int claim(uint32_t offset, uint32_t claims)
{
    int r;
    asm volatile( "mov r0, %2 \n" // assign r0 =  offset
                  "mov r1, %3 \n" // assign r1 =  claims
                  "svc %1     \n" // make system call SYS_CLAIM
                  "mov %0, r0 \n" // assign r  =    r0
                : "=r" (r) 
                : "I" (SYS_CLAIM), "r" (offset), "r" (claims)
                : "r0", "r1" );
    return r;
}

void release(uint32_t offset, uint32_t claims)
{
    asm volatile( "mov r0, %1 \n" // assign r0 =  offset
                  "mov r1, %2 \n" // assign r1 =  claims
                  "svc %0     \n" // make system call SYS_RELEASE
                :  
                : "I" (SYS_RELEASE), "r" (offset), "r" (claims)
                : "r0", "r1" );
}

void wait(pid_t pid)
{
    asm volatile( "mov r0, %1 \n" // assign r0 =  pid
                  "svc %0     \n" // make system call SYS_WAIT
                  :
                  : "I" (SYS_WAIT), "r" (pid)
                  : "r0" );
}

int inspect(pid_t pid)
{
    int r;
    asm volatile( "mov r0, %2 \n" // assign r0 =  pid
                  "svc %1     \n" // make system call SYS_INSPECT
                  "mov %0, r0 \n" // assign r  =    r0
                  : "=r" (r)
                  : "I" (SYS_INSPECT), "r" (pid)
                  : "r0" );
    return r;
}

int open(char* path)
{
    int r;
    asm volatile( "mov r0, %2 \n" // assign r0 =  path
                  //"mov r1, %3 \n" // assign r0 =  pathlength
                  "svc %1     \n" // make system call SYS_OPEN
                  "mov %0, r0 \n" // assign r  =    r0
                  : "=r" (r)
                  : "I" (SYS_OPEN), "r" (path)
                  : "r0");
    return r;
}

char* arg(int i)
{
    char* r;
    asm volatile( "mov r0, %2 \n" // assign r0 =  i
				  "svc %1     \n" // make system call SYS_ARG
                  "mov %0, r0 \n" // assign r  =    r0
                  : "=r" (r)
                  : "I" (SYS_ARG), "r" (i)
                  : "r0" );
    return r;
}

int filesize(int fd)
{
    int r;
    asm volatile( "mov r0, %2 \n" // assign r0 =  fd
                  "svc %1     \n" // make system call SYS_FILESIZE
                  "mov %0, r0 \n" // assign r  =    r0
                  : "=r" (r)
                  : "I" (SYS_FILESIZE), "r" (fd)
                  : "r0" );
    return r;
}
