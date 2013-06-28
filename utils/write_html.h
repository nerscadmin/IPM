
#ifndef WRITE_HTML_H_INCLUDED
#define WRITE_HTML_H_INCLUDED

#include <stdio.h>
#include "ipm_parse.h"

extern char buf1[128];
extern char buf2[128];

char *print_time(char *, int, struct timeval tv);
void write_html(FILE *f, job_t *job, banner_t *b);


#define JOBNAME(job)    basename(job->cmdline.c_str())
#define CMDPATH(job)    (job->cmdline.c_str())
#define CODENAME(job)   basename(job->cmdline.c_str())
#define MAXMEM(job,gs)  gs.dsum
#define USERNAME(job)   (job->username.c_str())
#define MPI_TASKS(job)  (job->ntasks)
#define HOST(job)       (job->hostname.c_str())
#define WALLCLOCK(job)  "wallclock"
#define GROUP(job)      "group"
#define COMM(job)       "comm"
#define STATE(job)      "completed"
#define GFLOP_SEC(job)  "gflops"
#define START(job)      print_time(buf1, 128, job->start) 
#define STOP(job)       print_time(buf2, 128, job->final) 


#endif /* WRITE_HTML_H_INCLUDED */
