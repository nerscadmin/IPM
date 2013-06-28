
#include <stdio.h>

int main() 
{
  FILE *f;

  f = fopen("test.dat", "w");
  fprintf(f, "Hello world\n");
  fclose(f);
}

