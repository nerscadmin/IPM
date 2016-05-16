#include "darshan-runtime-config.h"
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdarg.h>
#include <string.h>
#include <time.h>
#include <stdlib.h>
#include <errno.h>
#include <search.h>
#include <assert.h>
#define __USE_GNU
#include <pthread.h>

#include "uthash.h"

#include "darshan.h"
#include "darshan-dynamic.h"


/* structure to track i/o stats for a given hdf5 file at runtime */
struct ipm_runtime
{
    struct ipm_record record;
};

static struct ipm_runtime *ipm_runtime = NULL;
static pthread_mutex_t ipm_runtime_mutex = PTHREAD_RECURSIVE_MUTEX_INITIALIZER_NP;

/* the instrumentation_disabled flag is used to toggle functions on/off */
static int instrumentation_disabled = 0;

/* my_rank indicates the MPI rank of this process */
static int my_rank = -1;
static int darshan_mem_alignment = 1;

//forward declarations
static void ipm_runtime_initialize(); 
static void ipm_begin_shutdown();
static void ipm_get_output_data(MPI_Comm mod_comm, darshan_record_id *shared_recs, int shared_rec_count, void **buffer, int *size);
static void ipm_shutdown();

/* macros for obtaining/releasing the IPM module lock */
#define IPM_LOCK() pthread_mutex_lock(&ipm_runtime_mutex)
#define IPM_UNLOCK() pthread_mutex_unlock(&ipm_runtime_mutex)

/*
 * Function which inits the IPM record
 */
static void ipm_record_init(struct darshan_ipm_record *rec)
{
    rec->rank = my_rank;
    rec->fcounters[IPM_F_TIMESTAMP] = darshan_core_wtime();

    return;
}


/* initialize internal IPM module data strucutres and register with darshan-core */
static void ipm_runtime_initialize() 
{

    struct darshan_module_funcs ipm_mod_fns =
    {
        .begin_shutdown = &ipm_begin_shutdown,
        .get_output_data = &ipm_get_output_data,
        .shutdown = &ipm_shutdown
    };
    int mem_limit;
    char *recname = "darshan-internal-ipm";
	
	IPM_LOCK();
	
    /* don't do anything if already initialized or instrumenation is disabled */
    if(ipm_runtime || instrumentation_disabled)
        return;
	
    /* register the IPM module with the darshan-core component */
    darshan_core_register_module(
        DARSHAN_IPM_MOD,
        &ipm_mod_fns,
        &my_rank,
        &mem_limit,
        &darshan_mem_alignment);
		
	/* return if no memory assigned by darshan-core */
	if(mem_limit == 0)
	{
		instrumentation_disabled = 1;
		IPM_UNLOCK();
		return;
	}
	
    /* not enough memory to fit ipm module */
    if (mem_limit < sizeof(*ipm_runtime))
    {
        instrumentation_disabled = 1;
        IPM_UNLOCK();
        return;
    }

    /* initialize module's global state */
    ipm_runtime = malloc(sizeof(*ipm_runtime));
    if(!ipm_runtime)
    {
        instrumentation_disabled = 1;
        IPM_UNLOCK();
        return;
    }
    memset(ipm_runtime, 0, sizeof(*ipm_runtime));
	
    darshan_core_register_record(
        recname,
        strlen(recname),
        DARSHAN_IPM_MOD,
        1,
        0,
        &ipm_runtime->record.f_id,
        &ipm_runtime->record.alignment);

    /* if record is set to 0, darshan-core is out of space and will not
     * track this record, so we should avoid tracking it, too
     */
    if(ipm_runtime->record.f_id == 0)
    {
        instrumentation_disabled = 1;
        free(ipm_runtime);
        ipm_runtime = NULL;
        IPM_UNLOCK();
        return;
    }

    ipm_record_init(&ipm_runtime->record);

    IPM_UNLOCK();

    return;
}


/* Perform any necessary steps prior to shutting down for the IPM module. */
static void ipm_begin_shutdown()
{
    IPM_LOCK();

    /* In general, we want to disable all wrappers while Darshan shuts down. 
     * This is to avoid race conditions and ensure data consistency, as
     * executing wrappers could potentially modify module state while Darshan
     * is in the process of shutting down. 
     */
    instrumentation_disabled = 1;

    IPM_UNLOCK();

    return;
}
 

