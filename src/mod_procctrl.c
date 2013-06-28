
#define _GNU_SOURCE
#include <stdio.h>

#include <linux/unistd.h>
#include <execinfo.h>
#include <dlfcn.h>

#include "ipm.h"
#include "ipm_core.h"
#include "ipm_modules.h"


int mod_procctrl_init(ipm_mod_t* mod, int flags) 
{
  mod->state    = STATE_IN_INIT;
  mod->init     = mod_procctrl_init;
  mod->output   = 0;
  mod->finalize = 0; 
  mod->name     = "PROCCTRL";

  mod->state    = STATE_ACTIVE;
  return IPM_OK;
}

/* capture exit(), _exit(), exit_group() ... */

void exit(int status) {
  static int loaded=0;
  static void (*exit_real)(int status);
  
  if(!loaded) {
    exit_real=0;
 
    exit_real = (void (*)(int status)) 
      dlsym(RTLD_NEXT, "exit");

    if(!dlerror()) loaded=1; 
    else {
      IPMERR("Intercepting exit(): load failed\n");
    }
  }

  if( ipm_state==STATE_ACTIVE || ipm_state==STATE_NOTACTIVE ) 
    ipm_finalize(0);
  
  exit_real(status);
}

void _exit(int status) {
  static int loaded=0;
  static void (*_exit_real)(int status);

  if(!loaded) {
    _exit_real=0;
 
    _exit_real = (void (*)(int status)) 
      dlsym(RTLD_NEXT, "_exit");

    if(!dlerror()) loaded=1;
    else {
      IPMERR("Intercepting _exit(): load failed\n");
    }
  }

  if( ipm_state==STATE_ACTIVE || ipm_state==STATE_NOTACTIVE ) 
    ipm_finalize(0);


  _exit_real(status);
}

void exit_group(int status) {
  static int loaded=0;
  static void (*exit_group_real)(int status);

  if(!loaded) {
    exit_group_real=0;
 
    exit_group_real = (void (*)(int status)) 
      dlsym(RTLD_NEXT, "exit_group");

    if(!dlerror()) loaded=1;
    else {
      IPMERR("Intercepting exit_group(): load failed\n");
    }
  }

  if( ipm_state==STATE_ACTIVE || ipm_state==STATE_NOTACTIVE ) 
    ipm_finalize(0);
  
  exit_group_real(status);
}


pid_t fork(void) {
  static int loaded=0;
  static pid_t (*fork_real)(void);
  pid_t rv;
  
  if(!loaded) {
    fork_real=0;
    
    fork_real = (pid_t (*)(void))
      dlsym(RTLD_NEXT, "fork");
    
    if(!dlerror()) loaded=1;
    else {
      IPMERR("Intercepting fork(): load failed\n");
    }
  }

#if defined(HAVE_POSIXIO_TRACE) || defined(HAVE_MPI_TRACE)
  if( task.tracefile ) {
    fprintf(task.tracefile, "Before fork(), pid=%d\n", getpid());
  }
#endif
  
  rv = fork_real();
  
  if(!rv) {
    ipm_init(0);
#if defined(HAVE_POSIXIO_TRACE) || defined(HAVE_MPI_TRACE)
    task.tracefile=stderr;
#endif
  }

#if defined(HAVE_POSIXIO_TRACE) || defined(HAVE_MPI_TRACE)
  if( task.tracefile ) {
    fprintf(task.tracefile, "After fork(), pid=%d\n", getpid());
  }
#endif

  return rv;
}

pid_t vfork(void) {
  static int loaded=0;
  static pid_t (*vfork_real)(void);
  pid_t rv;

  if(!loaded) {
    vfork_real=0;

    vfork_real = (pid_t (*)(void))
      dlsym(RTLD_NEXT, "vfork");

    if(!dlerror()) loaded=1;
    else {
      IPMERR("Intercepting vfork(): load failed\n");
    }
  }

#if defined(HAVE_POSIXIO_TRACE) || defined(HAVE_MPI_TRACE)
  if( task.tracefile ) {
    fprintf(task.tracefile, "Before vfork(), pid=%d\n", getpid());
  }
#endif

  rv = vfork_real();

  if(!rv) {
    ipm_init(0);
#if defined(HAVE_POSIXIO_TRACE) || defined(HAVE_MPI_TRACE)
    task.tracefile=stderr;
#endif
  }

#if defined(HAVE_POSIXIO_TRACE) || defined(HAVE_MPI_TRACE)
  if( task.tracefile ) {
    fprintf(task.tracefile, "After vfork(), pid=%d\n", getpid());
  }
#endif

  return rv;
}
