#include "P10.h"

int isprime( uint32_t x ) {
  if ( !( x & 1 ) || ( x < 2 ) ) {
    return ( x == 2 );
  }

  for( uint32_t d = 3; ( d * d ) <= x ; d += 2 ) {
    if( !( x % d ) ) {
      return 0;
    }
  }

  return 1;
}

void main_P10()
{
    for( int i = 0; i < 25; i++ ) {
        write( STDOUT_FILENO, "P10", 3 );

        uint32_t lo = 1 <<  8;
        uint32_t hi = 1 << 16;

        for( uint32_t x = lo; x < hi; x++ ) {
            int r = isprime( x ); 
        }
    }
    exit(5);
}