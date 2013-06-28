/*
 *
 *   usage: ipm cmd
 *
 *   build: cc -o ipm -I. ipm.c 
 *
 *   David Skinner	Sep 2004 (dskinner@nersc.gov) 
 *   
 *   (poe|aprun|mpirun|mpiexec) ipm a.out
 *
 *    what does ./ipm do as opposed to libipm.a?
  
 *    It's an executable which manages things outside of MPI
 *     - set env variables like MP_EULIB_PATH
 *     - fork/exec follow HPM counts 
 *     - catch data from IPM_MPI upon exit?
 *     - control proc syscall tracing?
 *     - knows how to back off if MPI is available
 *     - process reports IPM generated log files
 *     - will tell you how long a second is
 *     - is a platform to accomplish things that can not be done easily
 *       from MPI such as reductions via disk or socket 
 *
 */

/* FIXME */
/* use vararg for fancy generic reduce */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/un.h>
#include <sys/stat.h>
#include <sys/utsname.h>
#include <string.h>
#include <signal.h>
#include <errno.h>
#include <netdb.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <ipm.h>

#ifdef OS_AIX
#include <sys/systemcfg.h>
#include <sys/processor.h>
#endif
 
#ifdef UNICOS_CRAYX1
#include <intrinsics.h>
#endif
                                                                                
#ifdef NEC_SX6
#include <time.h>
#endif
                                                                                
#define IPM_FILENAME_BASE  ".ipm"
#define TIMEOUT_init     5.0
#define TIMEOUT_barrier  5.0
#define IPM_POLL_USEC 1000

#define ipm_parallel_init_METHOD ipm_parallel_init_disk
#define ipm_parallel_barrier_METHOD ipm_parallel_barrier_disk
#define ipm_parallel_reduce_METHOD ipm_parallel_reduce_disk

#define IPM_IS_SERIAL      (0x0000000000000001ULL <<10)
#define IPM_IS_PARALLEL    (0x0000000000000001ULL <<11)
#define IPM_IS_MPI         (0x0000000000000001ULL <<12)
#define IPM_HPM_DO         (0x0000000000000001ULL <<13)
#define IPM_HPM_MULTIPLEX  (0x0000000000000001ULL <<14)
#define IPM_FAKE_COOKIE    (0x0000000000000001ULL <<15)
#define IPM_REPORT_DO      (0x0000000000000001ULL <<17)
#define IPM_REPORT_RUSAGE  (0x0000000000000001ULL <<18)
#define IPM_DATA_XML       (0x0000000000000001ULL <<19)
#define IPM_DATA_BINARY    (0x0000000000000001ULL <<20)
#define IPM_DATA_BIGEND    (0x0000000000000001ULL <<21)
#define IPM_DATA_LILEND    (0x0000000000000001ULL <<22)
#define IPM_PARSE_TERSE    (0x0000000000000001ULL <<30)
#define IPM_PARSE_FULL     (0x0000000000000001ULL <<31)
#define IPM_PARSE_HTML     (0x0000000000000001ULL <<32)

int ipm_listevents();
int ipm_timetest();
int ipm_parse(int argc, char *argv[]);
int ipm_parallel_init_disk(void);
int ipm_parallel_lock_disk(void);
int ipm_parallel_unlock_disk(void);
int ipm_parallel_barrier_disk(void);
int ipm_parallel_reduce_disk(char *tag, ...);

/*
int ipm_parallel_reduce_disk(char *tag, void *buf, int type, int root);
int ipm_parallel_bcast_disk(char *tag, void *buf, int type, int root);
int ipm_parallel_bcast_disk(char *tag, ...);
*/


unsigned long long int flags;
struct ipm_taskdata task;
struct ipm_jobdata job;

#include "ipm_env.c"
#include "ipm_init.c"
#include "ipm_finalize.c"
#include "ipm_trace.c"


/*
** Ephem to above
*/ 

int ipm_barrier_id=0,ipm_reduce_id=0;
int ipm_magic_offset=0;
struct flock ipm_fl = { F_WRLCK, SEEK_SET, 0,       0,     0 };
FILE *ipm_data_fh;
int ipm_data_fd;
static char ipm_fname[MAXSIZE_FILENAME];
static double wall, stamp;
double ipm_seconds_per_tick;
static struct timeval tod_tv;
pid_t ipm_pid;
double ipm_stamp_start;
char cbuf[MAXSIZE_LABEL], *env_ptr;
int fd[2];
int fd_rpfc, fd_rcfp;
int fd_wptc, fd_wctp;

#define MAXSIZE_TOKEN 5

int main (int argc, char *argv[]) {

 int i,j,k,zero=0,rv;
 char t_buf[MAXSIZE_TOKEN];
#ifndef IPM_DISABLE_UNAME
 struct utsname mach_info;
#endif

 ipm_init(0); 

 IPM_TIME_GTOD(ipm_stamp_start);
 ipm_pid = getpid();
 gethostname(task.hostname,MAXSIZE_LABEL);
#ifndef IPM_DISABLE_UNAME
 uname(&mach_info);
 sprintf(task.mach_info, "%s_%s", mach_info.machine, mach_info.sysname);
#else
 sprintf(task.mach_info,"IPM_DISABLE_UNAME");
#endif

 srand48(time(0));


 flags |= IPM_FAKE_COOKIE;
 flags |= IPM_REPORT_DO;
/*
 * getenv 
 * IPM_HPM=yes,[no]
 * IPM_MPI=yes,[no]
 */
 env_ptr = getenv("IPM_DEBUG");
 if(env_ptr) { flags |= DEBUG;}
 if(flags & DEBUG) {
  printf("IPM: %d got IPM_DEBUG\n", task.ipm_rank);
 }

 env_ptr = getenv("IPM_HPM");
 if(env_ptr && !strncmp(env_ptr,"no",2)) { flags &= ~IPM_HPM_DO;}

 /*
  * parallel_init : if parallel -> task.ipm_rank = 0..n-1, ipm_size=n
  *                   if serial   -> task.ipm_rank=0, ipm_size=-1
  */

 ipm_parallel_init_METHOD();

 if(flags & DEBUG) {
  for(i=0;i<argc;i++) {
   printf("IPM: %d pre-parse argc=%d argv[%d]=\"%s\"\n",task.ipm_rank,argc,i,argv[i]);
  }
 }

 if(argc < 2) {
  if(!task.ipm_rank) { /* print once parallel or serial */
   printf("usage: %s [-blrts] [-s n] program [[arg1] [arg2] [...]\n",argv[0]);
   printf("\n");
   printf("           -b bind to cpus/cores\n");
   printf("           -l list event sets\n");
   printf("           -r output getrusage data \n");
   printf("           -t test IPM timers\n");
   printf("           -s {1,2,3,4} select HPM event set\n");
   printf("\n");
   printf("parallel usage through MPI (prefered):\n");
   printf("\n");
   printf("       module load ipm | LD_PRELOAD \n");
   printf("\n");
/*
   printf("parallel usage when MPI not available:\n");
   printf("\n");
   printf("       mpirun -mpi_opts ipm [ipm_opts] prog [prog_opts]\n");
   printf("       poe -pe_opts  ipm [ipm_opts] prog [prog_opts]\n");
   printf("       aprun -mpi_opts ipm [ipm_opts] prog [prog_opts]\n");
   printf("\n");
*/
   printf("using ipm to parse a log file:\n");
   printf("\n");
   printf("       %s  -p      ipm_log_file(s)[...] parse \n",argv[0]);
   printf("       %s  -pfull  ipm_log_file(s)[...] parse w/ more detail\n",argv[0]);
   printf("       %s  -phtml ipm_log_file(s)[...] parse to html\n",argv[0]);
   printf("\n");
  }

  fflush(stdout);
  exit(1);
 }

 while(--argc && argv++) {
  if(!strcmp("-v",*argv)) {
   flags |= VERBOSE;
  } else if(!strcmp("-r",*argv)) {
   flags |= IPM_REPORT_RUSAGE;
  } else if(!strcmp("-t",*argv)) {
    --argc; argv++;
   ipm_timetest();
  } else if(!strcmp("-l",*argv)) {
   ipm_listevents();
  } else if(!strcmp("-p",*argv)) {
   flags |= IPM_PARSE_TERSE;
   ipm_parse(argc, argv);
  } else if(!strcmp("-pfull",*argv)) {
   flags |= IPM_PARSE_FULL;
   ipm_parse(argc, argv);
  } else if(!strcmp("-phtml",*argv)) {
   flags |= IPM_PARSE_HTML;
   ipm_parse(argc, argv);
  } else if(!strcmp("-mpi",*argv)) {
  } else if(!strcmp("-hpm",*argv)) {
  } else {
   break;
  }
 }

 if(flags & DEBUG) {
  for(i=0;i<argc;i++) {
   printf("IPM: %d post-parse argc=%d argv[%d]=\"%s\"\n",task.ipm_rank,argc,i,argv[i]);
  }
 }

 env_ptr = (char *)(malloc((size_t)MAXSIZE_LABEL*sizeof(char)));
 sprintf(env_ptr,"IPM_RANK=%d",task.ipm_rank); putenv(env_ptr);
 env_ptr = (char *)(malloc((size_t)MAXSIZE_LABEL*sizeof(char)));
 sprintf(env_ptr,"IPM_SIZE=%d",task.ipm_size); putenv(env_ptr);

 if(flags & DEBUG) {
  if(flags & VERBOSE) {printf("IPM: %d flags VERBOSE\n", task.ipm_rank);}
 }

 if((rv = pipe(fd)) != 0) { printf("IPM: %d ERROR pipe1 rv=%d\n", task.ipm_rank, rv); }
 fd_rpfc = fd[0]; 
 fd_wctp = fd[1];
 if(flags & DEBUG) {
  printf("IPM: %d pipe1 fd_rpfc %d fd_wctp %d\n", task.ipm_rank, fd[0], fd[1]);
 }

 if((rv = pipe(fd)) != 0) { printf("IPM: %d ERROR pipe2 rv=%d\n", task.ipm_rank, rv); }
 fd_rcfp = fd[0]; 
 fd_wptc = fd[1];
 if(flags & DEBUG) {
  printf("IPM: %d pipe2 fd_rcfp %d fd_wptc %d\n", task.ipm_rank, fd[0], fd[1]);
 }

 IPM_TIME_GTOD(task.stamp_start);
 if(flags & DEBUG) {
  printf("IPM: %d task fork %12.6f\n", task.ipm_rank, task.stamp_start);
 }
 task.pid = fork();
 switch(task.pid) {
  case 0:  /* child */
/*
   ptrace(PTRACE_TRACEME,NULL,NULL,NULL);
*/
   if(fd_wptc) close(fd_wptc);
   if(fd_rpfc) close(fd_rpfc);

   if(flags & DEBUG) {
    printf("IPM: %d child_go \"%s\"\n",task.ipm_rank, *argv);
   }

   read(fd_rcfp,t_buf,5);
   if(flags & DEBUG) {
    printf("IPM: %d child_go \"%s\"\n",task.ipm_rank, *argv);
   }
   rv = execvp(argv[0], argv+1);

   if(flags & DEBUG) {
    printf("IPM ERROR : execvp %s %s rv =%d\n", *argv, *argv, rv);
   }

   perror(*argv);

   exit(1);
   break;
  case -1: /* error parent, no child */
   if(errno==EAGAIN) { fprintf(stderr,"error: fork() EAGAIN\n"); exit(1); }
   if(errno==ENOMEM) { fprintf(stderr,"error: fork() ENOMEM\n"); exit(1); }
   fprintf(stderr,"error: fork() ?\n"); exit(1);
   break;
  default: /* parent */
   if(flags & DEBUG) {
    printf("IPM: %d wait child %d\n",task.ipm_rank, task.pid);
    fflush(stdout);
   }
   if(fd_wctp) close(fd_wctp);
   if(fd_rcfp) close(fd_rcfp);
   write(fd_wptc, "TOKEN", 5);
  }
/*
   while(1) {
    ptrace(PTRACE_SYSCALL,ZZ);
   }
*/

  /* signal() */
   waitpid(task.pid,0,0);
   IPM_TIME_GTOD(task.stamp_final);
   IPM_GETRUSAGE_CHILD(task.ru_SELF_final);

   if(flags & IPM_IS_PARALLEL) {
    ipm_parallel_barrier_METHOD();
   }


 /* on some MPI's (mpich2 w/ forker) any process finishing or exiting will 
    immediatley kill all other processes , so must wait
    this is similar the the issue of whether MPI_Finalize is globally 
    synchronizing */

 if(flags & IPM_IS_PARALLEL) {
  ipm_parallel_barrier_METHOD();
 }
 
 ipm_finalize();
 return(0);
}

int ipm_parallel_init_disk(void) {
 int i;
 char *cp, cbuf[MAXSIZE_FILENAME];
 char disk_hostname[MAXSIZE_FILENAME], disk_cookie[MAXSIZE_FILENAME];
 int fd,rv,n_before,left,right,done=0;
 int disk_size, disk_rank;
 pid_t disk_pid;
 double disk_stamp_start;
 double init_start, init_cur,  init_stop;
 double lock_start=-1, lock_stop=-1, lock_release=-1;

 IPM_TIME_GTOD(init_start);

 task.ipm_rank = 0; task.ipm_size = -1; /* key inition on ipm_size */

  if(flags & DEBUG) {
   printf("IPM: %d into ipm_parallel_init %d %d\n", task.ipm_rank, task.ipm_size, task.ipm_rank);
  }

 /* determine the number of tasks and rank */
#ifdef LINUX_X86
 cp = getenv("PMI_RANK"); 
 if(cp) { task.ipm_rank = atoi(cp); } else { task.ipm_rank = 0; }
 cp = getenv("PMI_SIZE"); 
 if(cp) { task.ipm_size = atoi(cp); } else { task.ipm_size = -1; }

#endif
#ifdef OS_AIX
 cp = getenv("MP_CHILD"); 
 if(cp) { task.ipm_rank = atoi(cp); } else { task.ipm_rank = 0; }
 cp = getenv("MP_PROCS"); 
 if(cp) { task.ipm_size = atoi(cp); } else { task.ipm_size = 0; }
#endif


 /* make an important decision */
 if(task.ipm_size > 0) {
  flags |= IPM_IS_PARALLEL; 
  if(flags & DEBUG) {
   printf("IPM: %d got IPM_IS_PARALLEL %d %d\n", task.ipm_rank, task.ipm_size, task.ipm_rank);
  }
 } else {
  if(flags & DEBUG) {
   printf("IPM: %d got IPM_IS_SERIAL %d %d\n", task.ipm_rank, task.ipm_size, task.ipm_rank);
  }
  return task.ipm_size;
 }

 /* only continue here if parallel, we still might cancel parallel later */
 
 /* get the job cookie */

 if(flags & IPM_IS_PARALLEL) {
#ifdef LINUX_X86
 cp = getenv("PBS_JOBCOOKIE");
#endif 
#ifdef OS_AIX
 cp = getenv("MP_PARTITION"); 
#endif 
 if(flags & IPM_FAKE_COOKIE) {
  cp = (char *)malloc((size_t)(20*sizeof(char)));
  sprintf(cp,"FAKECOOKIE");
 }
 }

 if(cp) {
  strncpy(job.cookie,cp,MAXSIZE_LABEL);
  if(flags & DEBUG) {
   printf("IPM: %d got job cookie %s\n",
	 task.ipm_rank,job.cookie);
  }
 } else {
   printf("IPM: %d got IPM_IS_PARALLEL but job cookie = %s\n",
	 task.ipm_rank, cp);
 }

  /* everyone open this file */
  sprintf(ipm_fname, "%s_%s", IPM_FILENAME_BASE, job.cookie);

  IPM_TIME_GTOD(lock_start);
  ipm_parallel_lock_disk();
  IPM_TIME_GTOD(lock_stop);
  if(flags & DEBUG) {
   printf("IPM: %d wait lock %.3e  %12.6f %12.6f\n",
	task.ipm_rank,
	lock_stop-lock_start,
	lock_start,
	lock_stop);
  }
 
  if(!ipm_data_fd) {
   perror("lock_fd");
   printf("IPM: %d ipm_data_fd is NULL \n", task.ipm_rank);
  }

  ipm_data_fh = fdopen(ipm_data_fd, "r+"); 
  if(!ipm_data_fh) {
   perror("fdopen");
   printf("IPM: %d can't fdopen %s(%d)\n", task.ipm_rank, ipm_fname,ipm_data_fd);
   return -1;
  }
  if(fileno(ipm_data_fh) != ipm_data_fd) {
   printf("IPM: %d file=\"%s\" fileno mismatch %d %d\n",
	 task.ipm_rank, ipm_fname,ipm_data_fd, fileno(ipm_data_fh));
  }
  fseek(ipm_data_fh,0,SEEK_SET);
  if(ferror(ipm_data_fh)) {
   printf("IPM: %d ferror \n", task.ipm_rank);
  }
  
  disk_size = disk_rank = disk_pid = disk_stamp_start = -1;
  n_before = 0;
  while( (rv = fscanf(ipm_data_fh, "%d %d %s %d %lf %s",
	 &disk_size,
	 &disk_rank,
	 disk_hostname,
	 &disk_pid,
         &disk_stamp_start,
	 disk_cookie))
	 == 6) {
   n_before ++; 
   if(flags & DEBUG) {
    printf("IPM: %d proci %s (%d %d) %d %d %d %f\n",
	task.ipm_rank,
	ipm_fname,
	n_before,
	rv,
	disk_size,
	disk_rank,
	disk_pid,
	disk_stamp_start);
   }
  }

   if(flags & DEBUG) {
    printf("IPM: %d proc  %s (%d %d) %d %d %d %f\n",
	task.ipm_rank,
	ipm_fname,
	n_before,
	rv,
	disk_size,
	disk_rank,
	disk_pid,
	disk_stamp_start);
   }

  switch(rv) {
   case 6:
   case EOF: {
    break;
   }
   case 1:
   case 2:
   case 3: 
   case 4: 
   case 5: {
    printf("IPM: %d corrupted data(%d) in  %s\n", task.ipm_rank, rv, ipm_fname);
    return -1;
    break;
   }
  }
  fseek(ipm_data_fh,0,SEEK_END);
  rv = fprintf(ipm_data_fh,"%d %8d %s %8d %12.6f %s\n",
	task.ipm_size,
	task.ipm_rank,
	task.hostname,
 	ipm_pid,
 	ipm_stamp_start,
 	job.cookie);

  if(flags & DEBUG) {
   printf("IPM: %d had %d before, wrote %d \n", task.ipm_rank,n_before, rv);
  }

   clearerr(ipm_data_fh);
   if(n_before == task.ipm_size-1) {
    fprintf(ipm_data_fh,"+++\nbarrier 0 0 \nreduce none 0\n");
  if(flags & DEBUG) {
   printf("IPM: %d wrote +++ \n", task.ipm_rank);
  }
   }
  fflush(ipm_data_fh); 
  /* release lock */
  ipm_parallel_unlock_disk();
  IPM_TIME_GTOD(lock_release);
  if(flags & DEBUG) {
   printf("IPM: %d held lock for %.3e %12.6f %12.6f\n",
	task.ipm_rank,
	lock_release-lock_stop,
	lock_stop,
	lock_release);
  }

  fflush(stdout);

  /* barrier on +++ */
  ipm_data_fh = fopen(ipm_fname, "r"); 
  if(!ipm_data_fh) {
   printf("IPM_ERROR : open barrier\n");
  }
  IPM_TIME_GTOD(init_cur);
  while(init_cur-init_start < TIMEOUT_init && !done) {
   usleep((int)(IPM_POLL_USEC*drand48()));
   IPM_TIME_GTOD(init_cur);
   fseek(ipm_data_fh,-31,SEEK_END);
/*
   i = fgetc(ipm_data_fh);
   printf("%c \n", i);
   ungetc(i,ipm_data_fh);
*/
   rv = fscanf(ipm_data_fh,"%s",cbuf);

   if(flags * DEBUG) {
    printf("IPM: %d init_poll+++ rv=%d cbuf=\"%s\"\n", task.ipm_rank,rv, cbuf);
   }

   if(rv == 1) {
    if(cbuf[0] == '+'  && cbuf[1] == '+'  && cbuf[2] == '+') {
     ipm_magic_offset = ftell(ipm_data_fh)+1;
     if(flags & DEBUG) {
      printf("IPM: %d got magic_offset=%d\n",
	 task.ipm_rank,
	 ipm_magic_offset);
     }
     done=1;
    }
   }
  }
  fclose(ipm_data_fh);

 if(flags & DEBUG) {
  printf("IPM: %d barrier offset %d\n",task.ipm_rank, ipm_magic_offset);
 }
 if(1) {
  return task.ipm_size;
 }
 printf("IPM: error in ipm_init\n");
 return -1;
}


int ipm_parallel_lock_disk(void) {
  int rv;
  ipm_data_fd = open(ipm_fname, O_CREAT|O_RDWR,0666);
  switch(ipm_data_fd) {
   case -1: {
    printf("IPM: %d can't open %s\n", task.ipm_rank, ipm_fname);
    break;
   }
   default: {
    break;
   }
  }
                                                                                
  if(flags & DEBUG) {
   printf("IPM: %d lock_file open succeeded fd = %d\n",
	 task.ipm_rank, ipm_data_fd);
  }

  /* now lock or block */
  ipm_fl.l_pid = ipm_pid;
/*
  ipm_fl.l_type = F_RDLCK;
*/
  if (fcntl(ipm_data_fd, F_SETLKW, &ipm_fl) == -1) {
   perror("fcntl");
   printf("IPM: %d can't lock %s\n", task.ipm_rank, ipm_fname);
   return -1;
  }
   if(flags & DEBUG) {
    printf("IPM: %d got lock %s\n", task.ipm_rank, ipm_fname);
   }
 return 0;
}

int ipm_parallel_unlock_disk(void) {
   int rv;

   ipm_fl.l_type = F_UNLCK;
   if (fcntl(ipm_data_fd, F_SETLK, &ipm_fl) == -1) {
    perror("fcntl");
   printf("IPM: %d can't unlock %s\n", task.ipm_rank, ipm_fname);
    return 1;
   }
   rv = fclose(ipm_data_fh);
   if(rv) {
    perror("fclose ipm_data_fh");
   }
   return 0;
}

int ipm_parallel_barrier_disk(void) {
 char disk_tag[MAXSIZE_LABEL];
 int rv, disk_barrier_id, disk_barrier_count;
 double t_start, t_current, t_final;
 char lbuf[80];


 if(flags & DEBUG) {
  printf("IPM: %d into barrier %d\n", task.ipm_rank, ipm_barrier_id);
 }

 IPM_TIME_GTOD(t_start);
/*
 if(flags & ~IPM_INITIALIZED) {
  printf("IPM: %d  ipm not initialized in barrier\n", task.ipm_rank);
 }
*/
/*
 while(fgets(lbuf,MAXSIZE_LABEL,ipm_data_fh)) { 
  printf("%d %s", task.ipm_rank, lbuf);
 }
*/

 /* arrive */
 IPM_TIME_GTOD(t_current);
 while(t_current-t_start < TIMEOUT_barrier) {
  ipm_parallel_lock_disk();
  ipm_data_fh = fdopen(ipm_data_fd, "r+");
  fseek(ipm_data_fh,ipm_magic_offset,SEEK_SET);
  if(flags & DEBUG) {
    rv=fgetc(ipm_data_fh);
    printf("IPM: %d fseek magic=%d char=%c\n",
	task.ipm_rank,
	ipm_magic_offset,
	(char)rv);
   ungetc(rv,ipm_data_fh);
  }

  rv = fscanf(ipm_data_fh,"%s %d %d",
	disk_tag,
	&disk_barrier_id,
	&disk_barrier_count);

  if(strcmp(disk_tag,"barrier")) {
   printf("IPM: %d ipm not aligned in barrier rv = %d \"%s\"\n",
	task.ipm_rank,
	rv,
	disk_tag);
  }

  if(disk_barrier_id == ipm_barrier_id) {
   fseek(ipm_data_fh,ipm_magic_offset,SEEK_SET);
   if(disk_barrier_count == task.ipm_size-1) {
   rv = fprintf(ipm_data_fh,"barrier %d %d",disk_barrier_id+1,disk_barrier_count+1);
   } else {
    rv = fprintf(ipm_data_fh,"barrier %d %d",disk_barrier_id,disk_barrier_count+1);
   }
   /*
   printf("%d wrote %d\n", task.ipm_rank, disk_barrier_count+1);
   */
   fflush(ipm_data_fh);
   ipm_barrier_id++;
   IPM_TIME_GTOD(t_final);
   if(flags & DEBUG) {
    printf("IPM: %d out of barrier %d\n", task.ipm_rank, ipm_barrier_id);
   }
   break;
  }

  if(disk_barrier_id < ipm_barrier_id) {
   /* not all tasks are at the same barrier */
   /* so wait for others to catch up via the while loop above */
  }
  ipm_parallel_unlock_disk();
  IPM_TIME_GTOD(t_current);
 }
 
 /* wait for arrivals */

  ipm_data_fh = fopen(ipm_fname, "r");
  if(!ipm_data_fh) {
   printf("IPM_ERROR : open wait arrive barrier\n");
  }
  IPM_TIME_GTOD(t_current);
  while(t_current-t_start < TIMEOUT_barrier) {
  fseek(ipm_data_fh,ipm_magic_offset,SEEK_SET);
  rv = fscanf(ipm_data_fh,"%s %d %d",
	disk_tag,
	&disk_barrier_id,
	&disk_barrier_count);
   if(flags & DEBUG) {
    printf("IPM: %d poll_barrier rv=%d tag=\"%s\" %d %d\n",
	 task.ipm_rank,
	 rv,
	 disk_tag,
	disk_barrier_id,
	disk_barrier_count);
   }
   if(disk_barrier_count == task.ipm_size) {
    fclose(ipm_data_fh);
    return 0;
   }
   usleep((int)(IPM_POLL_USEC*drand48()));
  IPM_TIME_GTOD(t_current);
  }
  fclose(ipm_data_fh);


 if(flags & DEBUG) {
  printf("IPM: %d timeout in barrier %d\n", task.ipm_rank, ipm_barrier_id);
 }
 
 return 0;
}

int ipm_parallel_reduce_disk(char *tag, ...) {
 return 0;
}

int show_proctree(void) {
 int ndescend=0,done=0;
 char cmd[MAXSIZE_LABEL];
 pid_t p,pp;
 pid_t q,qq;
 FILE *fh;

 p = ipm_pid; /* or ipm_task_pid */
 pp = getppid();
 while(!done) {
  sprintf(cmd, "/bin/ps --noheaders --pid %d -o \"%%P %%p %%a\"",p);
  fh = popen(cmd,"r");
  if(p!=q) printf("proctree err : q!=p\n");
  fscanf(fh,"%d %d", &qq,&q);
  fclose(fh);
  if(qq==1) done = 1;
  printf("->%d", p);
  p = q; 
 }
  
  printf("\n");

 return ndescend;
}

int ipm_timetest() {
 IPM_TICK_TYPE T1,T2;
 long int usec;
 double wall;

#ifndef LINUX_BGL
 IPM_TIME_INIT;

/* need IPM_TIME_RDTSC_SECPERTICK */

 for(usec=0;usec<1000000;usec += usec/4+1) {
  IPM_TIME_BRACKET(usleep(usec));
  wall =    IPM_TIME_SEC(T2)-IPM_TIME_SEC(T1),

  printf("IPM: timetest usec %lld count %.10e sec_per_tick %.10e\n",
   (long long int)usec, wall, 
   (1.0e-6*usec)/wall);
 }

 fflush(stdout);
#endif

 return 0;
}

int ipm_parse(int argc, char *argv[]) {

 FILE *log_fh;
 char cbuf[MAXSIZE_TXTLINE];
 char *fname;
 int	rv;


 fname = argv[argc-1]; 

 log_fh = fopen(fname, "r");

 if(!log_fh) {
  printf("IPM: can't open logfile \"%s\" to parse \n", fname);
  exit(1);
 }

 /* detect if the file is XML text or binary */

 rv = fscanf(log_fh,"%c5.5", cbuf); 
 
 if(rv == EOF) {
  printf("IPM ERROR : EOF on %s, file truncated or unrecognized\n", argv[argc-1]);
  exit(1);
 } 

 if(rv == 1 && strncmp(cbuf,"<?xml",5)==0 ) {
  flags |= IPM_DATA_XML;
 } else if(rv == 1 && strncmp(cbuf,"<?XML",5)==0 ) {
  flags |= IPM_DATA_XML;
 } else {
  flags |= IPM_DATA_BINARY;
 }

 printf("###############################################################################\n");
 printf("###############################################################################\n");

 exit(0);
 return 0;
}
 
int ipm_listevents() {
 ipm_hpm_test(1);
 exit(0);
}

#include "ipm_hpm.c"
#include "ipm_memusage.c"
#include "ipm_execinfo.c"
#include "ipm_jobinfo.c"
#include "ipm_switchinfo.c"
#include "ipm_util.c"

   /*     
   ** fin  
   */

