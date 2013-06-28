#include <stdio.h>
#include <stdlib.h>
#include <sched.h>
#include <string.h>
#include <time.h>
#include <fcntl.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <sys/types.h>
#include <unistd.h>
#include <errno.h>
#ifdef MPI
#include <mpi.h>
#endif

int flags=0;

#define VERBOSE1      (1<< 0)
#define VERBOSE2      (1<< 1)
#define BIND          (1<< 2)
#define LABELIO       (1<< 3)
#define REPORT_NONE   (1<<10)
#define REPORT_TERSE  (1<<11)
#define REPORT_FULL   (1<<12)
#define REPORT_XML    (1<<13)
#define BOUND_TIME    (1<<15)
#define BOUND_MEM     (1<<16)

#define MAXBUF 1760
#define MAXSTR 4096

void usage (char *argv[]);
void nonblock(int sock);

typedef struct task_data {
 pid_t pid;
 double ti,tf;
 double m_maxrss, m_heap_init, m_heap_final;
 int pc[2],cp[2],cpu,coff;
 struct rusage ru;
 char cbuf[MAXBUF];
} task_data;

/* plumb

 parent writes to child on  pc[1]
 child reads from parent on pc[0]
 child writes to parent on  cp[1]
 parent reads from child on cp[0]

*/

#ifdef LINUX
unsigned long aff_mask=1;
unsigned int len = sizeof(aff_mask);
#define CPU(i) ( aff_mask << i)
#define BIND_CPU(p,i) {\
	 aff_mask = (1 << i);\
         if (sched_setaffinity(p, len, &aff_mask)) {\
	  printf("sched_setaffinity(%d,%d,%ld)\n", p,(int)len,aff_mask);\
	  perror("sched_setaffinity failed");\
          fflush(stdout);\
	  exit(1);\
	 }\
}
#define GOT_BIND 1
#endif

#ifndef GOT_BIND 
#warning process binding unavailble try -D{LINUX}
#endif


int main(int argc, char *argv[]) {
    int n=-1,ndone=0,i,j,itask,slen;
    struct timeval tv;
    pid_t wpid;
    task_data *task;
    struct rusage rub;
    int pstatus;
    int maxfd;
    fd_set fdrset,fdwset;
    char *cp;
    char *env_str;
    char *env_label_id=NULL;
#ifdef MPI
    int mpi_rank,mpi_size;
#endif


    if(argc==1) { usage(argv); exit(0); }

#ifdef MPI
    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &mpi_rank);
    MPI_Comm_size(MPI_COMM_WORLD, &mpi_size);
#endif

    while(--argc && argv++) {
     if(!strcmp("-v",*argv)) {
      flags |= VERBOSE1;
     } else if(!strcmp("-vv",*argv)) {
      flags |= VERBOSE1;
      flags |= VERBOSE2;
     } else if(!strcmp("-h",*argv)) {
      usage(argv); exit(0);
     } else if(!strcmp("--help",*argv)) {
      usage(argv); exit(0);
     } else if(!strcmp("-time",*argv)) { /* unimplemented time box */
       --argc; argv++;
       flags |= BOUND_TIME;
     } else if(!strcmp("-mem",*argv)) { /* unimplemented mem box */
       --argc; argv++;
       flags |= BOUND_MEM;
     } else if(!strcmp("-labels",*argv)) {
      flags |= LABELIO;
     } else if(!strcmp("-n",*argv)) {
       --argc; argv++;
       n = atoi(*argv);
     } else if(!strncmp("--env_id=",*argv,9)) {
       env_label_id = *argv+9;
     } else {
       break;
     }
    }



    if(n<=0) { n = 1; } /* default n=1 */

    if(flags & VERBOSE1) {
     printf("# n = %d cmd = \"%s ", n, *argv);
     for(i=1;i<argc;i++) {
      printf("%s ", *(argv+i));
     }
     printf("\"\n");
    }

    task = (task_data *)malloc((size_t)(n*sizeof(task_data)));
    env_str = (char *)malloc((size_t)(MAXSTR*sizeof(char)));

    if(flags & VERBOSE2) {
     printf("#about to launch %d tasks\n", n);
    }

    for(i=0;i<n;i++) {

    
      pipe(task[i].pc);
      pipe(task[i].cp);
      if(flags & VERBOSE1) {
       printf("#pipe i=%d pc read=%d write=%d\n",i,task[i].pc[0],task[i].pc[1]);
       printf("#pipe i=%d cp read=%d write=%d\n",i,task[i].cp[0],task[i].cp[1]);
      }

     if(env_label_id) {
      snprintf(env_str,MAXSTR,"%s=%d",env_label_id,i);
     } else {
      slen=snprintf(env_str,MAXSTR,"VRUN_TASKID=%d",i);
     }
     putenv(env_str);

     task[i].pid = fork(); 

     switch(task[i].pid) {

	case 0: /* child */

         close(1); dup(task[i].cp[1]);
         close(0); dup(task[i].pc[0]);

         if(flags & VERBOSE1) {
          printf("# execvp(%s",*argv);
          if(argc > 1) {
           printf(",");
	   for(i=1;i<argc;i++) {
            printf("%s,",*(argv+i));
           }
          }
          printf(")\n");
         }

	 execvp(*argv,argv);
	 printf("execvp failed : errno = %d\n",errno);
         exit(-1);
         break;

	case -1: /* error parent, no child */
	 if(errno==EAGAIN) {fprintf(stderr,"error: fork() EAGAIN\n"); exit(1); }
	 if(errno==ENOMEM) {fprintf(stderr,"error: fork() ENOMEM\n"); exit(1); }
	 fprintf(stderr,"error: fork() ?\n"); sleep(100);
	 break;

	default: /* parent */
         close(task[i].cp[1]); /* close child to parent write */
         close(task[i].pc[0]); /* close parent to child read */
	 gettimeofday(&tv,NULL);
	 task[i].ti = tv.tv_sec+1.0e-6*tv.tv_usec;
         if(flags & VERBOSE1) {
          printf("# task %d pid %d start %.12f\n", i, task[i].pid, task[i].ti);
         }
#ifdef GOT_BIND
         if(flags & BIND) {
	  BIND_CPU(task[i].pid,i);
         }
#endif
	 break;
     }
    }

/* parent code */

    maxfd = 0;
    for(i=0;i<n;i++) {
     maxfd = ((task[i].cp[0] > maxfd)?(task[i].cp[0]):(maxfd));
     memset(task[i].cbuf,0,MAXBUF);
     task[i].coff = 0;
    }
    maxfd++;




    FD_ZERO(&fdrset);
    ndone = 0;
    while(ndone <= n-1) {

     for(i=0;i<n;i++) {
      FD_SET(task[i].cp[0], &fdrset);
     }
     select(maxfd,&fdrset, NULL,NULL,NULL);
     itask = -1;
     for(i=0;i<n;i++) {
      if (FD_ISSET(task[i].cp[0], &fdrset)) {
       itask = i;
       task[i].coff = strlen(task[i].cbuf);
       j = read(task[i].cp[0],
	        task[i].cbuf+strlen(task[i].cbuf),
	        MAXBUF-strlen(task[i].cbuf));
       if(flags & VERBOSE2) {
        printf("#read j,i %d,%d len %d-> %s\n",j,i, strlen(task[i].cbuf), task[i].cbuf);
       }
       while((cp=strchr(task[i].cbuf,'\n')) != NULL) {
        j = strlen(task[i].cbuf);
        *cp = '\0';

        if(flags & VERBOSE2) {
         printf("#%d len %d %d : --%s--\n", i, strlen(task[i].cbuf),j, task[i].cbuf);
        }

        printf("%d :: %s\n", i,task[i].cbuf);
        memmove(task[i].cbuf,cp+1,j-strlen(task[i].cbuf));
       }
      }
     }
     if(itask < 0) {
      printf("ERR:\n");
     }
     if(flags & VERBOSE2) {
      printf("#children complete %d\n", ndone);  
     }

     wpid = wait4(-1,&pstatus,WNOHANG,&rub);

     if(wpid < 0)  break;
     if(wpid > 0) {
      gettimeofday(&tv,NULL);
      for(j=0;j<n;j++) {
       if(task[j].pid == wpid) break;
      }
      task[j].ru = rub;
      task[j].tf = tv.tv_sec+1.0e-6*tv.tv_usec;
      ndone ++; ndone --;
      if(flags & VERBOSE1) {
       printf("# task %d pid %d finish %.12f\n", j, task[j].pid, task[j].tf);
      }
     }


     if(flags & VERBOSE2) {
       printf("#children complete %d\n", ndone);  
     }
    }

    if(flags & REPORT_NONE) {
    } else if(flags & REPORT_TERSE) {
    } else if(flags & REPORT_FULL) {
    } else if(flags & REPORT_XML) {
    }

    return 0;

}

void report(void) {
/*

<xml>
<vrun>
<perf> </perf>
<task id=0> </task>
</vrun>

##############################################################################
# cmd=
##############################################################################

*/

}

void usage (char *argv[]) {
       fprintf(stderr, "usage: %s -n N [opts] {cmd [args]| -f cmdfile}\n", argv[0]);
       fprintf(stderr, "\n");
       fprintf(stderr, "\t-n N\trun N tasks (default N=1)\n");
       fprintf(stderr, "\t-v|--verbose\tshow details about what vrun is doing\n");
       fprintf(stderr, "\t-vv|--vverbose\tshow more details about what vrun is doing\n");
       fprintf(stderr, "\t-b,--bind=X\tbind tasks to CPUs in pattern X={default,folded}\n");
       fprintf(stderr, "\t-l,--labelio\tprepend task id to each line of output\n");
       fprintf(stderr, "\t-o file\t\twrite stdout to file\n");
       fprintf(stderr, "\t--iter=N\teach task runs cmd N times\n");
       fprintf(stderr, "\t--report=X\tlevel of reported detail X={none,terse,full,xml}\n");
       fprintf(stderr, "\t--env_id=VAR\tset environment VAR to task's numerical id\n");
       fprintf(stderr, "\t--tag=STR\tprepend STR to report lines\n");
       fprintf(stderr, "\n");
       fprintf(stderr, "The goal of vrun is to provide a versatile way to run things. Ideas welcome.\n");
#ifndef GOT_BIND
       fprintf(stderr, "\n");
       fprintf(stderr, "(process binding is unavailble as currently compiled, see -DLINUX,-DAIX)\n ");
       fprintf(stderr, "\n");
#endif
    }

