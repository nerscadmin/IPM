
#ifndef MOD_CLUSTERING_H_INCLUDED
#define MOD_CLUSTERING_H_INCLUDED

#include "ipm_modules.h"

typedef struct 
{
  int   myrank;    /* own rank */
  int   clrank;    /* rank of the cluster representative */
  
  /* 
   *  'structural' or algorithmic metrics
   */
  int ncoll;       /* number of collective ops */
  int nroot;       /* number of times as root of the op */
  int np2p;        /* number of p2p ops */
  int npar;        /* number of comm. partners in p2p ops */
  double parloc;   /* 'location' of comm. partners */ 
  double pardist;  /* 'distance' of comm. partners */

  /* 
   * quantitative metrics
   */
  double wallt;     /* wallclock time */
  double gflops;    /* gigaflop rate */
  double tcoll;     /* time in collectives */
  double tp2p;      /* time in p2p */
  double dcoll;     /* data transferred in collectives */
  double dp2p;      /* data transferred in p2p */

} procstats_t;


extern procstats_t mystats;

int mod_clustering_init(ipm_mod_t* mod, int flags);
int mod_clustering_output(ipm_mod_t* mod, int flags);

void print_procstat(int hdr, procstats_t *stats);

#endif /* MOD_CLUSTERING_H_INCLUDED */
