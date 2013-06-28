
#include <unistd.h>
#include <sys/types.h>
#include <errno.h>
#include <stdio.h>
#include <sys/wait.h>
#include <stdlib.h>
 

int main()
{
  pid_t childpid; 
  int retval;
  int status;
  
  childpid = fork();
  
  fopen("/dev/null", "r");
  
  if (childpid >= 0) /* fork succeeded */
    {
      if (childpid == 0) /* fork() returns 0 to the child process */
        {
	  fopen("/dev/null", "r");

	  fprintf(stderr, "CHILD: pid=%d\n", getpid());
	  abort();
	  sleep(1); /* sleep for 1 second */


        }
      else /* fork() returns new pid to the parent process */
        {
	  fopen("/dev/null", "r");
	  fopen("/dev/null", "r");
	  fprintf(stderr, "PARENT: pid=%d\n", getpid());
	  sleep(1);
        }
    }
  else 
    {
      perror("fork"); /* display error message */
      exit(0); 
    }
}

