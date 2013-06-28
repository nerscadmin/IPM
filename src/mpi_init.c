
#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "ipm.h"
#include "ipm_core.h"
#include "perfdata.h"
#include "hashkey.h"
#include "hashtable.h"
#include "machtopo.h"
#include "GEN.calltable_mpi.h"
#include "GEN.fproto.mpi.h"

#include "mod_mpi.h"

#ifdef HAVE_KEYHIST
#include "mod_keyhist.h"
#endif

#define FINIT_CALLS_CINIT
/* #define GETARGS_FROM_PROC */

void ipm_mpi_init() {
  unsigned idx, idx2;
  unsigned csite;
  IPM_KEY_TYPE key;
  double tstart, tstop, t;

  /* --- */
  PMPI_Comm_rank( MPI_COMM_WORLD, &(task.taskid) );
  PMPI_Comm_size( MPI_COMM_WORLD, &(task.ntasks) );
  PMPI_Comm_group( MPI_COMM_WORLD, &ipm_world_group );

  ipm_get_machtopo();

  IPMDBG("ipm_mpi_init rank=%d size=%d\n", 
	 task.taskid, task.ntasks);

  if( !(task.flags&FLAG_LOGWRITER_POSIXIO) && 
      !(task.flags&FLAG_LOGWRITER_MPIIO) ) {
    
    if( task.ntasks<=256 )
      task.flags|=FLAG_LOGWRITER_POSIXIO;
    else 
      task.flags|=FLAG_LOGWRITER_MPIIO;
  }


#ifdef HAVE_CALLPATH
  csite=get_callsite_id();
#else
  csite=0;
#endif
 
  IPM_MPI_KEY(key, MPI_INIT_ID_GLOBAL, 0, 0, 1, csite);
  IPM_HASH_HKEY(ipm_htable, key, idx);

#ifdef HAVE_MPI_TRACE
#ifdef HAVE_KEYHIST 
  KEYHIST_TRACE(task.tracefile,key);
#else
  if( task.tracefile && task.tracestate) {
    fprintf(task.tracefile, "%s %d %d %d %d\n",
	    "MPI_Init", 0, 0, 0, csite);
  }
#endif /* HAVE_KEYHIST */
#endif /* HAVE_MPI_TRACE */

#ifdef HAVE_KEYHIST
  KEY_ASSIGN(last_hkey,key);
  last_tstamp=ipm_wtime();
#endif

  ipm_htable[idx].count++;
  ipm_htable[idx].t_min=0.0;
  ipm_htable[idx].t_max=0.0;
  ipm_htable[idx].t_tot=0.0;
  /* --- */
}


void getargs(int* ac, char** av, int max_args)
{
  int i, pid, exelen, insize;
  char *inbuf, file[256];
  FILE* infile;
  char* arg_ptr;
  long fsize;
  
  *ac = 0;
  *av = NULL;
  i=0; insize=256;

  pid = getpid();
  sprintf(file, "/proc/%d/cmdline", pid);
  infile = fopen(file, "r");
  
  if( infile != NULL ) {
    while ( !feof(infile) ) {
      inbuf = IPM_MALLOC(4096);
      if ( fread(inbuf, 1, 4096, infile) > 0 ) {
	arg_ptr = inbuf;
	while ( *arg_ptr != 0 ) {
	  av[i] = strdup(arg_ptr);
	  arg_ptr += strlen(av[i]) + 1;
	  i++;
	}
      }
    }
    *ac = i;
    
    if( inbuf ) IPM_FREE(inbuf);
    fclose(infile);
  }
}

int MPI_Init(int *argc, char ***argv)
{
  int rv;
  char buf[80];
  int myrank;

  ipm_init(0);
  
  rv = PMPI_Init(argc, argv);
  
  PMPI_Comm_rank( MPI_COMM_WORLD, &myrank);
#if defined(HAVE_POSIXIO_TRACE) || defined(HAVE_MPI_TRACE) || defined(HAVE_OMP_TRACE) 
  if( !task.tracefile ) {
    sprintf(buf, "%s.trace.%d.txt", task.fname, myrank);
    task.tracestate=0;
    task.tracefile=fopen(buf, "w");
    task.tracestate=1;
  }
#endif

#if defined(HAVE_POSIXIO)
  modules[IPM_MODULE_POSIXIO].state=STATE_ACTIVE;
#endif 

  ipm_mpi_init();

  return rv;
}

int MPI_Init_thread(int *argc, char ***argv, int requested, int *provided)
{
  int rv;
  char buf[80];
  int myrank;

  ipm_init(0);

  rv = PMPI_Init_thread(argc, argv, requested, provided);
  
  PMPI_Comm_rank( MPI_COMM_WORLD, &myrank);
#if defined(HAVE_POSIXIO_TRACE) || defined(HAVE_MPI_TRACE) || defined(HAVE_OMP_TRACE)
    if( !task.tracefile ) {
    sprintf(buf, "%s.trace.%d.txt", task.fname, myrank);
    task.tracestate=0;
    task.tracefile=fopen(buf, "w");
    task.tracestate=1;
  }
#endif
  
  ipm_mpi_init();

  return rv;  
}

void MPI_INIT_F(int* ierr)
{ 
  int argc;
  char **argv[32];
  static char *name="./foo";

#ifdef FINIT_CALLS_CINIT
  argc=1;
  argv[0]=&name;
  *ierr = MPI_Init(&argc, argv);
#else 
  ipm_init(0);
  PMPI_INIT_F(ierr); 
  ipm_mpi_init();
#endif  /* FINIT_CALLS_CINIT */
}


void MPI_INIT_THREAD_F(int *request, int *provide, int *ierr)
{ 
  int argc;
  char **argv[32];
  static char *name="./foo";

#ifdef FINIT_CALLS_CINIT
  argc=1;
  argv[0]=&name;
  *ierr = MPI_Init_thread(&argc, argv, *request, provide);
#else 
  ipm_init(0);
  PMPI_INIT_THREAD_F(ierr); 
  ipm_mpi_init();
#endif  /* FINIT_CALLS_CINIT */
}
