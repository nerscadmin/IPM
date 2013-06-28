
#include <stdlib.h>
#include <libgen.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <time.h>

#include "ipm.h"
#include "ipm_core.h"
#include "ipm_time.h"
#include "perfdata.h"
#include "jobdata.h"
#include "md5.h"
#include "regstack.h"


taskdata_t task;

void taskdata_init(taskdata_t *t) 
{
  char *tmp;

  gettimeofday( &(t->t_start), 0);

  t->flags = FLAG_REPORT_TERSE|FLAG_LOG_TERSE;

  t->wtime    = ipm_wtime();
  t->stime    = ipm_stime();
  t->utime    = ipm_utime();
  t->mtime    = ipm_mtime();
  t->iotime   = ipm_iotime();
  t->omptime  = ipm_omptime();

  t->procmem = 0.0;

  gethostname(t->hostname, MAXSIZE_HOSTNAME);
  t->hostname[MAXSIZE_HOSTNAME-1]=0;

  t->pid=getpid();

  t->taskid=0;
  t->ntasks=1;
  t->nhosts=1;

  ipm_get_job_id(t->jobid, MAXSIZE_JOBID);
  ipm_get_job_user(t->user, MAXSIZE_USERNAME);
  ipm_get_job_allocation(t->allocation, MAXSIZE_ALLOCATIONNAME);

  ipm_get_mach_name(t->mach_name, MAXSIZE_MACHNAME);
  ipm_get_mach_info(t->mach_info, MAXSIZE_MACHINFO);

  ipm_get_exec_cmdline(t->exec_cmdline, t->exec_realpath);
  ipm_get_exec_md5sum(t->exec_md5sum, t->exec_realpath);

  /*
   * determine local appname and filename prefix 
   * later root broadcasts its settings such that
   * that they are consisten
   */

  /* need copy, because basename may modify arg */
  tmp = strdup(t->exec_realpath); 
  sprintf(t->appname, "%s", basename(tmp));
  sprintf(t->fname, "%s.%lu", t->user, t->t_start.tv_sec);
  free(tmp);

  sprintf(t->logdir, ".");

#if defined(HAVE_POSIXIO_TRACE) || defined(HAVE_MPI_TRACE) 
  t->tracestate=1;
  t->tracefile=0;
#endif 

#if defined(HAVE_SNAP) 
  t-> snap_period = 0;
  t-> snap_last = 0;
  mkdir("/tmp/ipm",S_IWOTH|S_IROTH|S_IWGRP|S_IRGRP|S_IWUSR|S_IRUSR|S_IXUSR|S_IXGRP|S_IXOTH);
#endif

  t->rstack = ipm_rstack;
}


