
#ifdef HAVE_SELFMONITOR

#include "ipm.h"
#include "ipm_core.h"
#include "ipm_sizes.h"
#include "ipm_debug.h"
#include "ipm_modules.h"
#include "ipm_types.h"
#include "hashtable.h"
#include "report.h"
#include "mod_selfmonitor.h"

#include <stdlib.h>

#define STRINGIFY(x) #x
#define TOSTRING(x) STRINGIFY(x)

#define FOO asfd

selfmon_t ipm_selfmon;

int mod_selfmonitor_output(struct ipm_module* mod, int flags);

int mod_selfmonitor_init(struct ipm_module* mod, int flags)
{
  int i, comp, rv;

  mod->state    = STATE_IN_INIT;
  mod->init     = mod_selfmonitor_init;
  mod->output   = mod_selfmonitor_output;
  mod->finalize = 0; 
  mod->name     = "SELFMON";

  ipm_selfmon.mem_alloc=0;
  ipm_selfmon.num_alloc=0;
  ipm_selfmon.num_free=0;
  ipm_selfmon.bytes_send=0;
  ipm_selfmon.bytes_recv=0;
  ipm_selfmon.num_mpi=0;
  ipm_selfmon.t_mpi_p2p=0.0;
  ipm_selfmon.t_mpi_coll=0.0;
  ipm_selfmon.t_finalize=0.0;
  ipm_selfmon.t_init=0.0;

  mod->state    = STATE_ACTIVE;
  return IPM_OK;
}


int mod_selfmonitor_output(struct ipm_module* mod, int flags)
{
  gstats_t ginit, gfinalize, gmpi_p2p, gmpi_coll;
  gstats_t gfill, galloc, gsend, grecv;
  gstats_t gnalloc, gnfree;
  double fill, alloc;
  double send, recv;
  IPM_COUNT_TYPE nalloc, nfree;
  int i;

  ipm_selfmon.t_finalize = ipm_wtime()-ipm_selfmon.t_finalize;
  fill = 100.0*(MAXSIZE_HASH-ipm_hspace)/(double)MAXSIZE_HASH;
  alloc = ipm_selfmon.mem_alloc/(double)(1024);
  send = ipm_selfmon.bytes_send/(double)(1024);
  recv = ipm_selfmon.bytes_recv/(double)(1024);

  nalloc = ipm_selfmon.num_alloc;
  nfree  = ipm_selfmon.num_free;

  gstats_double( ipm_selfmon.t_init, &ginit );
  gstats_double( ipm_selfmon.t_finalize, &gfinalize );
  gstats_double( ipm_selfmon.t_mpi_p2p, &gmpi_p2p );
  gstats_double( ipm_selfmon.t_mpi_coll, &gmpi_coll );

  gstats_double( fill, &gfill );
  gstats_double( alloc, &galloc );
  
  gstats_count( nalloc, &gnalloc );
  gstats_count( nfree, &gnfree );

  gstats_double( send, &gsend );
  gstats_double( recv, &grecv );

  if( task.taskid==0 ) {
    fprintf(stderr, "##IPM2 SELF-MONITORING ############################################\n");
    fprintf(stderr, "#\n");
    fprintf(stderr, "#   modules : ");
    for( i=0; i<MAXNUM_MODULES; i++ ) {
      if( modules[i].name ) 
	fprintf(stderr, "%s%s ", modules[i].name, 
		(modules[i].state==STATE_ACTIVE?"(on)":"(off)"));
    }
    fprintf(stderr, "\n");
    fprintf(stderr, "#\n");
    fprintf(stderr, "#   types   : IPM_COUNT_TYPE : '" TOSTRING(IPM_COUNT_TYPE) "' (%lu bytes)\n", 
	    sizeof(IPM_COUNT_TYPE));
    fprintf(stderr, "#           : IPM_ADDR_TYPE  : '" TOSTRING(IPM_ADDR_TYPE) "' (%lu bytes)\n",
	    sizeof(IPM_ADDR_TYPE));
    fprintf(stderr, "#           : IPM_RANK_TYPE  : '" TOSTRING(IPM_RANK_TYPE) "' (%lu bytes)\n",
	    sizeof(IPM_RANK_TYPE));
    fprintf(stderr, "#\n");

    fprintf(stderr, "#   sizes   : MAXNUM_MODULES     : %d\n", MAXNUM_MODULES);
    fprintf(stderr, "#           : MAXSIZE_FILENAME   : %d\n", MAXSIZE_FILENAME);
    fprintf(stderr, "#           : MAXSIZE_CMDLINE    : %d\n", MAXSIZE_CMDLINE);
    fprintf(stderr, "#           : MAXSIZE_HOSTNAME   : %d\n", MAXSIZE_HOSTNAME);
    fprintf(stderr, "#\n");

    fprintf(stderr, "# #defines  : MPI_STATUS_COUNT      = %s\n", 
	    TOSTRING(MPI_STATUS_COUNT));

#ifdef OS_LINUX
    fprintf(stderr, "#           : OS_LINUX               yes\n"); 
#else
    fprintf(stderr, "#           : OS_LINUX               no\n");
#endif

#ifdef DELAYED_MPI_FINALIZE
    fprintf(stderr, "#           : DELAYED_MPI_FINALIZE   yes\n"); 
#else
    fprintf(stderr, "#           : DELAYED_MPI_FINALIZE   no\n");
#endif
    fprintf(stderr, "#\n");

    fprintf(stderr, "#   atexit  : handler %s\n", 
	    (task.flags&FLAG_USING_ATEXIT)?"installed":"not installed");

    fprintf(stderr, "# logwriter : %s\n", 
	    (task.flags&FLAG_LOGWRITER_POSIXIO)?"POSIXIO":
	    (task.flags&FLAG_LOGWRITER_MPIIO)?"MPIIO":"UNDEFINED");

    fprintf(stderr, "#\n");
    fprintf(stderr, "# hashtable : %d entries [%lu bytes], %d available on task 0\n",
	    MAXSIZE_HASH, sizeof(ipm_htable), ipm_hspace);
    fprintf(stderr, "#           : %lu bit keys, total size of hash entry is %lu bytes\n", 
	    sizeof(IPM_KEY_TYPE)*8, sizeof(ipm_hent_t));
    fprintf(stderr, "#           :\n");
    fprintf(stderr, "#           :      [total]         <avg>         min          max\n");
    fprintf(stderr, "# tinit     :   %10.2f   %10.2f   %10.2f   %10.2f \n", 
	    ginit.dsum, ginit.dsum/(double)task.ntasks, ginit.dmin, ginit.dmax);
    fprintf(stderr, "# tfinalize :   %10.2f   %10.2f   %10.2f   %10.2f \n", 
	    gfinalize.dsum, gfinalize.dsum/(double)task.ntasks, gfinalize.dmin, gfinalize.dmax);
    fprintf(stderr, "# tmpi_p2p  :   %10.2f   %10.2f   %10.2f   %10.2f \n",
	    gmpi_p2p.dsum, gmpi_p2p.dsum/(double)task.ntasks, gmpi_p2p.dmin, gmpi_p2p.dmax);
    fprintf(stderr, "# tmpi_coll :   %10.2f   %10.2f   %10.2f   %10.2f \n",
	    gmpi_coll.dsum, gmpi_coll.dsum/(double)task.ntasks, gmpi_coll.dmin, gmpi_coll.dmax);
    fprintf(stderr, "# hfill [%%] :                %10.2f   %10.2f   %10.2f \n",
	    gfill.dsum/(double)task.ntasks, gfill.dmin, gfill.dmax);
    fprintf(stderr, "# alloc[KB] :                %10.2f   %10.2f   %10.2f \n",
	    galloc.dsum/(double)task.ntasks, galloc.dmin, galloc.dmax);
    fprintf(stderr, "# nalloc    :                %10"IPM_COUNT_TYPEF"   %10"
	    IPM_COUNT_TYPEF"   %10" IPM_COUNT_TYPEF "\n", 
	    gnalloc.nsum/task.ntasks, gnalloc.nmin, gnalloc.nmax);
    fprintf(stderr, "# nfree     :                %10"IPM_COUNT_TYPEF"   %10"
	    IPM_COUNT_TYPEF"   %10" IPM_COUNT_TYPEF "\n", 
	    gnfree.nsum/task.ntasks, gnfree.nmin, gnfree.nmax);
    fprintf(stderr, "# send [KB] :                %10.2f   %10.2f   %10.2f \n",
	    gsend.dsum/(double)task.ntasks, gsend.dmin, gsend.dmax);
    fprintf(stderr, "# recv [KB] :                %10.2f   %10.2f   %10.2f \n",
	    grecv.dsum/(double)task.ntasks, grecv.dmin, grecv.dmax);
    fprintf(stderr, "#\n");
    fprintf(stderr, "###################################################################\n");

    /* 
       IPMMSG("num_alloc=%u\n", ipm_selfmon.num_alloc);
       IPMMSG("num_free=%u\n", ipm_selfmon.num_free);
       IPMMSG("num_mpi=%u\n", ipm_selfmon.num_mpi);
    */
  }

  return IPM_OK;
}

void *ipm_calloc(size_t nmemb, size_t size)
{
  void *ptr;

  ipm_selfmon.num_alloc++;
  ipm_selfmon.mem_alloc+=(nmemb*size);

  ptr = calloc(nmemb, size);

  IPMDBG("MEM calloc %p %lu %lu\n", ptr, 
	 (unsigned long)nmemb, (unsigned long)size);

  return ptr;
}

void *ipm_malloc(size_t size)
{
  void *ptr;

  ipm_selfmon.num_alloc++;
  ipm_selfmon.mem_alloc+=size;

  ptr = malloc(size);

  IPMDBG("MEM malloc %p %lu\n", ptr, (unsigned long)size);
  
  return ptr;
}

void ipm_free(void *ptr)
{
  ipm_selfmon.num_free++;
  IPMDBG("MEM free %p\n", ptr);

  free(ptr);
}

void *ipm_realloc(void *ptr, size_t size)
{
  void *ret;

  ipm_selfmon.num_alloc++;
  ipm_selfmon.mem_alloc+=size;

  ret=realloc(ptr,size);
 
  IPMDBG("MEM realloc %p %p %lu\n", ret, ptr, (unsigned long)size);

  return ret;
}


#ifdef HAVE_MPI
#include <mpi.h>

int ipm_bcast(void *buffer, int count, MPI_Datatype datatype, int root, 
	      MPI_Comm comm) 
{
  int rv; 
  double time;
  
  time=ipm_wtime();
  rv=PMPI_Bcast(buffer, count, datatype, root, comm);
  
  ipm_selfmon.t_mpi_coll+=(ipm_wtime()-time);
  ipm_selfmon.num_mpi++;

  return rv;
}

int ipm_allgather(void *sendbuf, int sendcount, MPI_Datatype sendtype,
                  void *recvbuf, int recvcount, MPI_Datatype recvtype, 
		  MPI_Comm comm) 
{
  int rv;
  double time;
  
  time=ipm_wtime();

  rv = PMPI_Allgather(sendbuf, sendcount, sendtype,
		      recvbuf, recvcount, recvtype, comm);
  ipm_selfmon.t_mpi_coll+=(ipm_wtime()-time);
  ipm_selfmon.num_mpi++;

  return rv;
}

int ipm_gather(void *sendbuf, int sendcount, MPI_Datatype sendtype,
	       void *recvbuf, int recvcount, MPI_Datatype recvtype, 
	       int root, MPI_Comm comm) 
{
  int rv;
  double time;
  
  time=ipm_wtime();

  rv = PMPI_Gather(sendbuf, sendcount, sendtype,
		   recvbuf, recvcount, recvtype, root, comm);

  ipm_selfmon.t_mpi_coll+=(ipm_wtime()-time);
  ipm_selfmon.num_mpi++;

  return rv;
}

int ipm_reduce(void *sendbuf, void *recvbuf, int count,
	       MPI_Datatype datatype, MPI_Op op, int root, MPI_Comm comm) 
{
  int rv;
  double time;
  
  time=ipm_wtime();
  
  rv = PMPI_Reduce(sendbuf, recvbuf, count, datatype, op, root, comm);

  ipm_selfmon.t_mpi_coll+=(ipm_wtime()-time);
  ipm_selfmon.num_mpi++;

  return rv;
}

int ipm_allreduce(void *sendbuf, void *recvbuf, int count, 
		  MPI_Datatype datatype, MPI_Op op, MPI_Comm comm) 
{
  int rv;
  double time;
  
  time=ipm_wtime();

  rv = PMPI_Allreduce(sendbuf, recvbuf, count, 
		      datatype, op, comm);
  
  ipm_selfmon.t_mpi_coll+=(ipm_wtime()-time);
  ipm_selfmon.num_mpi++;
  
  return rv;
}


int ipm_send(void *buf, int count, MPI_Datatype datatype, int dest, 
	     int tag, MPI_Comm comm) 
{
  int rv;
  double time;
  int dsize;
  
  PMPI_Type_size(datatype, &dsize);
  dsize*=count;

  time=ipm_wtime();
  rv = PMPI_Send(buf, count, datatype, dest, tag, comm);
  ipm_selfmon.t_mpi_p2p+=(ipm_wtime()-time);
  ipm_selfmon.num_mpi++;
  ipm_selfmon.bytes_send+=dsize;

  return rv;
}


int ipm_recv(void *buf, int count, MPI_Datatype datatype, int source, 
	     int tag, MPI_Comm comm, MPI_Status *status) 
{
  int rv;
  double time;
  int dsize;

  PMPI_Type_size(datatype, &dsize);
  dsize*=count;

  time=ipm_wtime();
  rv = PMPI_Recv(buf, count, datatype, source, tag, comm, status);
  ipm_selfmon.t_mpi_p2p+=(ipm_wtime()-time);  
  ipm_selfmon.num_mpi++;
  ipm_selfmon.bytes_recv+=dsize;

  return rv;

}

#endif /* HAVE_MPI */



#endif /* HAVE_SELFMONITOR */

