
/** HEADER_BEGIN **/

#define _GNU_SOURCE
#include <sys/types.h>
#include <unistd.h>
#include <stdio.h>

#include "cuda_runtime_api.h"
#include "cuda.h"

/** HEADER_END **/


__CRET__ __real___CFNAME__(__CPARAMS__)
{
#if __RETURN_VALUE__
  return 0;
#endif
}

