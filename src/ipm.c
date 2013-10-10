#include <stdio.h>
#include <stdlib.h>
#include <sched.h>
#include <time.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <sys/types.h>
#include <unistd.h>
#include <errno.h>


int flags=0;

#define VERBOSE       (1<< 0)
#define BIND          (1<< 1)
#define LABELIO       (1<< 2)
#define REPORT_NONE   (1<<10)
#define REPORT_TERSE  (1<<11)
#define REPORT_FULL   (1<<12)
#define REPORT_XML    (1<<13)

#define MAXSIZE_ENV 4096

void usage (char *argv[]);
void nonblock(int sock);

typedef struct task_data {
 pid_t pid;
 double ti,tf;
 double m_maxrss, m_heap_init, m_heap_final;
 int pc[2],cp[2],cpu;
 struct rusage ru;
} task_data;

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

#ifdef AIX
#define GOT_BIND 2
#endif

#ifndef GOT_BIND 
#warning process binding unavailble try -D{LINUX|AIX}
#endif

int main(int argc, char *argv[]) {
    int n=-1,ndone=0,i,j;
    struct timeval tv;
    pid_t wpid;
    task_data *task;
#ifdef GOT_RUSAGE
    struct rusage rub;
#endif
    int pstatus;
    fd_set fdlist;
    char env_str[MAXSIZE_ENV];


    if(argc==1) { usage(argv); exit(0); }

    while(--argc && argv++) {
     if(!strcmp("-v",*argv)) {
      flags |= VERBOSE;
     } else if(!strcmp("-h",*argv)) {
      usage(argv); exit(0);
     } else if(!strcmp("--help",*argv)) {
      usage(argv); exit(0);
     } else if(!strcmp("-l",*argv)) {
      flags |= LABELIO;
     } else if(!strcmp("-n",*argv)) {
       --argc; argv++;
       n = atoi(*argv);
     } else if(!strcmp("-np",*argv)) {
       --argc; argv++;
       n = atoi(*argv);
     } else {
       break;
     }
    }


    if(n<=0) { n = 1; }


    if(flags & VERBOSE) {
     printf("# n = %d cmd = \"%s ", n, *argv);
     for(i=1;i<argc;i++) {
      printf("%s ", *(argv+i));
     }
     printf("\"\n");
    }

    task = (task_data *)malloc((size_t)(n*sizeof(task_data)));

    for(i=0;i<n;i++) {

     if(flags & LABELIO) {
      pipe(task[i].pc);
      pipe(task[i].cp);
      if(flags & VERBOSE) {
       printf("#pipe i=%d pc read=%d write=%d\n",i,task[i].pc[0],task[i].pc[1]);
       printf("#pipe i=%d cp read=%d write=%d\n",i,task[i].cp[0],task[i].cp[1]);
      }
     }

     sprintf(env_str,"VRUN_TASKID=%d",i);
     putenv(env_str);


     task[i].pid = fork(); 

     switch(task[i].pid) {

	case 0: /* child */

         if(flags & LABELIO) {
          close(1); dup(task[i].cp[1]);
          close(0); dup(task[i].pc[0]);
         }

         if(flags & VERBOSE) {
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
         if(flags & LABELIO) {
          close(task[i].cp[1]); /* close child to parent write */
          close(task[i].pc[0]); /* close parent to child read */
         }
	 gettimeofday(&tv,NULL);
	 task[i].ti = tv.tv_sec+1.0e-6*tv.tv_usec;
         if(flags & VERBOSE) {
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

/*
    if(flags & LABELIO) {
     maxfd = 0;
     for(i=0;i<n;i++) {
      maxfd = ((task[i].cp[0] > maxfd)?(task[i].cp[0]):(maxfd))
     }
     maxfd = ++;
     FD_ZERO(&fd_read_set);
    }

*/

/* { */

    ndone = 0;
    while(ndone < n-1) {

     printf("it %d\n", ndone); 

#ifdef GOT_RUSAGE
     wpid = wait4(-1,&pstatus,WNOHANG,&rub);
#else 
     wpid = waitpid(-1,&pstatus,0);
     if(0) wpid = wait(&pstatus); 
#endif

     if(wpid < 0)  break;
     if(wpid > 0) {
      gettimeofday(&tv,NULL);
      for(j=0;j<n;j++) {
       if(task[j].pid == wpid) break;
      }
#ifdef GOT_RUSAGE
      task[j].ru = rub;
#endif
      task[j].tf = tv.tv_sec+1.0e-6*tv.tv_usec;
      ndone ++;
      if(flags & VERBOSE) {
       printf("# task %d pid %d finish %.12f\n", j, task[j].pid, task[j].tf);
      }
     }


     printf("bit %d\n", ndone); 
    }

    if(flags & REPORT_NONE) {
    } else if(flags & REPORT_TERSE) {
    } else if(flags & REPORT_FULL) {
    } else if(flags & REPORT_XML) {
    }

    return 0;

/* } */

}

void nonblock(int sock) {
        int opts;

        if(flags & VERBOSE) {
         printf("# nonblock sock = %d\n", sock);
        }

        if(!sock) {
         printf("error nonblock sock = %d\n", sock);
         exit(1);
        }

        opts = fcntl(sock,F_GETFL);
        if (opts < 0) {
                perror("fcntl(F_GETFL)");
                exit(EXIT_FAILURE);
        }
        opts = (opts | O_NONBLOCK);
        if (fcntl(sock,F_SETFL,opts) < 0) {
                perror("fcntl(F_SETFL)");
                exit(EXIT_FAILURE);
        }
        return;
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
       fprintf(stderr, "usage: %s [-n|-np] N [opts] {cmd [args]| -f cmdfile}\n", argv[0]);
       fprintf(stderr, "\n");
       fprintf(stderr, "\t-n|-np N\trun N tasks\n");
       fprintf(stderr, "\t-v|--verbose\tshow details about what vrun is doing\n");
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

