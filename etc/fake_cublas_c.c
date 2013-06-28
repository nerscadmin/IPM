
/** HEADER_BEGIN **/

#define _GNU_SOURCE
#include <sys/types.h>
#include <unistd.h>
#include <stdio.h>

#include "cuda.h"
#include "cublas.h"

/** HEADER_END **/


__CRET__ __real___CFNAME__(__CPARAMS__)
{
#if __RETURN_VALUE__
  __CRET__ rv;
  return rv;
#endif
}

