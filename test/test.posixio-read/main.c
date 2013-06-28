
#include <stdio.h>

int main() 
{
  char buf[80];
  FILE *f;

  f = fopen("/dev/urandom", "r");

  fread( buf, 80, 1, f );
  fgetc( f );
  fread( buf, 80, 1, f );
  fgetc( f );

  fclose(f);
}

