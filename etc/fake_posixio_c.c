
/** HEADER_BEGIN **/

#define _GNU_SOURCE
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdio.h>

#define MPI3CONST const

/** HEADER_END **/


__CRET__ __real___CFNAME__(__CPARAMS__)
{
#if __RETURN_VALUE__
  return 0;
#endif
}

