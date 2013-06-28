#ifndef IPM_INTROSPECT_H_INCLUDED
#define IPM_INTROSPECT_H_INCLUDED

/*
 * PIA == Performance Introspection API 
 *
 */

/* for compile-time checks */
#define IPM_HAVE_PIA  1


#define PIA_MAXLEN_LABEL 64

/* return values for functions */
typedef int pia_ret_t;

#define PIA_OK         0
#define PIA_NOTFOUND  -1


/*
 * pia_regid_t is an integer identifier for regions
 *
 * == 0  represents the whole application
 *  < 0  invalid, does not exist, error condition
 *  > 0  a valid user-defined region in IPM
 */
typedef int pia_regid_t;


typedef struct 
{
  pia_regid_t  id;
  char         name[PIA_MAXLEN_LABEL];
  unsigned     count; /* executed how many times? */
  double       wtime; /* wallclock time */
  double       mtime; /* time in mpi */
} pia_regdata_t;


/* 
 * navigate the region hierarchy:
 * 
 * - pia_current_region() returns the id of the region at 
 *   the point of invocation 
 * - pia_child_region() returns the id of the *first* 
 *   sub (child) region
 * - pia_parent_region() returns the id of the 
 *   parent of the current region
 * - pia_next_region() returns the next region on the 
 *   same level of the hierarchy
 * 
 * negative return values indicate that the requested region does not
 * exist
 */

pia_regid_t pia_current_region(void); 
pia_regid_t pia_child_region(pia_regid_t reg);
pia_regid_t pia_parent_region(pia_regid_t reg);
pia_regid_t pia_next_region(pia_regid_t reg);

pia_regid_t pia_find_region_by_name(char *name);

pia_ret_t pia_get_region_data(pia_regdata_t *rdata, pia_regid_t reg);

/*
 * pia_act_t is an integer identifier for IPM activities 
 * an activity is like an MPI or Posix-IO call or time spent
 * inside an OpenMP region
 *
 * => 0  represents a valid activity
 * < 0 represents error, not available, ...
 */
typedef int pia_act_t;

/* #define PIA_ACT_ALL_MPI     0xFFFF */

pia_act_t pia_find_activity_by_name(char *name);


typedef struct 
{
  int ncalls;
  double tmin, tmax, tsum;
} pia_actdata_t; 

pia_ret_t pia_init_activity_data(pia_actdata_t *adata);
pia_ret_t pia_get_activity_data(pia_actdata_t *adata, pia_act_t act);


#endif /* IPM_INTROSPECT_H_INCLUDED */
