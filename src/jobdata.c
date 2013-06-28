
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <pwd.h>
#include <grp.h>
#include <sys/types.h>
#include <sys/utsname.h>
#include <sys/stat.h>

#include "ipm_sizes.h"
#include "md5.h"

void ipm_get_job_id(char *id, int len)
{
  char *s=0;
  
  s = getenv("PBS_JOBID"); 
  if(!s) s = getenv("LOADL_STEP_ID");
  if(!s) s = getenv("SLURM_JOBID");
  if(!s) s = getenv("JOB_ID");
  if(!s) s = getenv("LSB_JOBID");
  if(!s) {
    strncpy(id, "unknown", len);
  } else {
    strncpy(id, s, len);
  }

}

void ipm_get_job_user(char *user, int len) 
{
  
  char *s=0;
  s = getenv("USER");
  if(s) {
    strncpy(user, s, len);
  } else {
    strncpy(user, "unknown", len);
  }
}

void ipm_get_job_allocation(char *allocation, int len) 
{
  char *s=0; 
  
  
  if(!s) s = getenv("REPO");
  if(!s) s = getenv("GROUP");
  if(s) {
    sprintf(allocation, "%s", s);
  } else {
    strncpy(allocation, "unknown", len);
  }

}


void ipm_get_mach_info(char *machi, int len)
{
  char buf[200];
  struct utsname mach_info;

#ifndef IPM_DISABLE_UNAME
  uname(&mach_info);
  sprintf(buf, "%s_%s", 
	  mach_info.machine, mach_info.sysname);
  strncpy(machi, buf, len);
#else 
  strncpy(machi, "unknown", len);
#endif
}


void ipm_get_mach_name(char *machn, int len)
{
  char buf[200];
  struct utsname mach_info;

#ifndef IPM_DISABLE_UNAME
  uname(&mach_info);
  sprintf(buf, "%s", mach_info.machine); 
  strncpy(machn, buf, len);
#else 
  strncpy(machn, "unknown", len);
#endif
}


void ipm_get_exec_cmdline(char *cmdl, char *rpath) 
{
  int i, ii, fd, rv, blen;
  FILE *fh;
  char *cp,*pp,*up,cbuf[MAXSIZE_CMDLINE];

  cmdl[0] = '\0';
  fh = fopen("/proc/self/cmdline","r"); 
  if(fh) {
    /*
      rv=fscanf(fh,"%s",cmdl);
      printf("rv %d cmdl %s\n", rv, cmdl);
      rv=fscanf(fh,"%s",cmdl);
      printf("rv %d cmdl %s\n", rv, cmdl);
    */
    ii = 0;
    fgets(cmdl,MAXSIZE_CMDLINE,fh);
    for(i=1;i<MAXSIZE_CMDLINE;i++)  {
      if(cmdl[i]==0 && ii == 0) {
	cmdl[i]=' '; ii = 1;
      } else if(cmdl[i]==0 && ii == 1) {
	break;
      } else {
	ii = 0;
      }
    }
    fclose(fh);
  } else {
    sprintf(cmdl, "unknown");
  }
 
  rv=readlink("/proc/self/exe",rpath,MAXSIZE_CMDLINE); 
  if(rv < 1) sprintf(rpath,"unknown");

}

void ipm_get_exec_md5sum(char *exec_md5sum, char *rpath) {

  FILE *fh;
  int i;
  unsigned sbuf[16];

  if(!strcmp(rpath,"unknown")) {
   sprintf(exec_md5sum,"unknown"); 
  } else {
   fh = fopen(rpath,"rb");
   md5_stream(fh,sbuf);
   fclose(fh);
   for (i = 0; i < 16; ++i) sprintf (exec_md5sum+2*i,"%02x", sbuf[i]);
  }


}

