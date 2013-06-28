
#include <stdarg.h>
#include <stdio.h>

#include "ipm_parse.h"

int IPM_DIAG(job_t *job, const char* format, ...)
{
  int rv=0;
  va_list ap;

  if( !(job->quiet) ) {
    va_start (ap,format);
    vfprintf(stderr, format, ap);
  }
  
  return rv;
}
