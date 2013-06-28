
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#ifdef HAVE_MPI
#include <mpi.h>
#endif /*HAVE_MPI*/

#include "perfdata.h"
#include "hashtable.h"
#include "mod_callpath.h"
#include "mod_selfmonitor.h"

void init_cstable(cstable_t *t, int size)
{
  t->ncs=size;
  t->csids = IPM_CALLOC(size, sizeof(int));
  t->cstable = IPM_CALLOC(size*MAXSIZE_CALLSTACKDEPTH,
		      sizeof(void*));
}

void clear_cstable(cstable_t *t)
{
  int i, j;
  for( i=0; i<t->ncs; i++ ) {
    t->csids[i]=0;
    for( j=0; j<MAXSIZE_CALLSTACKDEPTH; j++ ) {
      t->cstable[i*MAXSIZE_CALLSTACKDEPTH+j]=0;
    }
  }
}


void* cgfunc_store_leaves(callsite_t *site, int level, int flags, void *ptr)  
{
  if( flags==VISIT_BACKTRACK )
    return ptr;
  
  if( IS_NODE(site)&&IS_LEAF(site) ) {
    (*((void**)ptr))=site;
    return ((char*)ptr)+sizeof(void*);
  }

  return ptr;
}

void* cgfunc_store_nodes(callsite_t *site, int level, int flags, void *ptr)  
{
  if( flags==VISIT_BACKTRACK )
    return ptr;
  
  if( IS_NODE(site) ) {
    (*((void**)ptr))=site;
    return ((char*)ptr)+sizeof(void*);
  }

 return ptr;
}

void* cgfunc_remap_ids(callsite_t *site, int level, int flags, void *ptr) 
{
  int *map;

  if( flags==VISIT_BACKTRACK )
    return ptr;

  map=(int*)ptr;
  if( site->addr && map[site->id] ) {
    site->id=map[site->id];
  }
  return ptr;
}



void callgraph_to_cstable(callgraph_t *g, cstable_t *table)
{
  int i, j, k;
  void **nodes;
  int nnodes;
  void **cstack[MAXSIZE_CALLSTACKDEPTH];
  callsite_t *node;

  //nnodes  = callgraph_count_leaves(g);
  nnodes  = callgraph_count_nodes(g);

  nodes = IPM_CALLOC(nnodes, sizeof(void*));
  init_cstable(table, nnodes);

  /* retrieve address of leaves */ 
  callgraph_traverse(g, 0, cgfunc_store_nodes, nodes);
  //  callgraph_traverse(g, 0, cgfunc_store_leaves, nodes);
  
  for( i=0; i<nnodes; i++ ) {
    memset(cstack, 0, sizeof(void*)*MAXSIZE_CALLSTACKDEPTH);

    node = ((callsite_t*)nodes[i]); j=0;
    table->csids[i]=node->id;
    
    while(node) {
      cstack[j++] =(void*)(node->addr);	
      node=node->parent;
    }
    
    /* reverse order (start with root) */
    for( k=0; k<j; k++  ) {
      table->cstable[i*MAXSIZE_CALLSTACKDEPTH+k]=cstack[j-1-k];
    }
  } 
}


void print_cstable(FILE *f, cstable_t *table)
{
  int i, j;
  
  for( i=0; i<table->ncs; i++ ) {
    fprintf(f, "%5u: ", table->csids[i]);
    
    for( j=0; j<MAXSIZE_CALLSTACKDEPTH; j++ ) {
      if( !(table->cstable[i*MAXSIZE_CALLSTACKDEPTH+j]) )
	break;
      fprintf(f, "%p ", table->cstable[i*MAXSIZE_CALLSTACKDEPTH+j]);
    }
    //    fprintf(f, "%s ", callsite2callname(table->csids[i]));
    fprintf(f, "\n");
  }
}


/* compare two stacks, return 0 if equal */
int stackcmp(void **s1, void **s2, int len) {
  int i;
  
#if defined(HAVE_BACKTRACE) || defined(HAVE_LIBUNWIND)
  for( i=0; i<len; i++ ) {
    if( i==1 && (*(s1+i)==0&&*(s2+i)!=0) ||
	(*(s1+i)!=0&&*(s2+i)==0) ) 
      return 1;
       
    if( i!=1 && *(s1+i)!=*(s2+i) ) {
      return 1;
    }
  }
#else
  for( i=0; i<len; i++ ) {
    if( *(s1+i)!=*(s2+i) ) {
      return 1;
    }
  }    
#endif

  return 0;
}

void stackcpy(void *src, void *dest, int len) {
  bcopy(src, dest, sizeof(void*)*len);
}


/* see if ID id is unused in this table, if not find one that is */
int find_free_id(cstable_t *t, int id) {
  int done, i;

  done=0;
  while(!done) {
    done=1;
    for(i=0; i<t->ncs; i++ ) {
      if( id==(t->csids[i]) ) {
	done=0; id++;	
	break;
      }
    }
  }
  return id;
}

/*
 * merge t2 into t1 and provide remap table for CSids for t2
 */
void merge_cstables(cstable_t *t1, cstable_t *t2, int *remap )
{
  void *s1, *s2;
  int i, j, k;
  int found;
    
  for( k=0; k<t2->ncs; k++ ) {
    /* try to find stack k of t2 in t1... */
    found=0;
    for( i=0; i<t1->ncs; i++ ) {
      s1 = &((t1->cstable)[i*MAXSIZE_CALLSTACKDEPTH]);
      s2 = &((t2->cstable)[k*MAXSIZE_CALLSTACKDEPTH]);

      if( !stackcmp(s1, s2, MAXSIZE_CALLSTACKDEPTH) ) {
	found=1;
	break;
      }
    }
    if( found /* at i */) {
      /*      fprintf(stderr, "found %d at %d re-map %u->%u \n", k, i, 
	      t2->csids[k], t1->csids[i]); 
      */
      if( remap ) remap[t2->csids[k]]=t1->csids[i];
      continue;
    }

    /* try to find empty slot */
    found=0;
    for( i=0; i<t1->ncs; i++ ) {
      if( t1->cstable[i*MAXSIZE_CALLSTACKDEPTH]==0 ) {
	found=1;
	break;
      }
    }
    if( found /* empty slot at i */) {
      //   fprintf(stderr, " %u found free slot at %d\n", t2->csids[k], i); 
      s1 = &((t1->cstable)[i*MAXSIZE_CALLSTACKDEPTH]);
      s2 = &((t2->cstable)[k*MAXSIZE_CALLSTACKDEPTH]);

      t1->csids[i] = find_free_id(t1, t2->csids[k]);
      if( remap ) remap[t2->csids[k]]=t1->csids[i];
      stackcpy(s2, s1, MAXSIZE_CALLSTACKDEPTH);
    }
    else {
      /* handle error (t1 not big enough) */
    }
  }
}

void remap_callsites(ipm_hent_t *table, int *map, int maxid)
{
  int i, cs;

  callgraph_traverse(ipm_callgraph, 0, cgfunc_remap_ids, map);

  htable_remap_callsites(table, map, maxid);

#ifdef HAVE_KEYHIST
  xhtable_remap_callsites(table, map, maxid);
#endif
}


#ifdef HAVE_MPI

void ipm_unify_callsite_ids()
{
  /* 1. build the local cstable 
     2. send it to root where it is merged and unified
     3. root sends out mapping table
     4. apply maps to callgraph 
  */
  cstable_t local, remote, merged;
  MPI_Status status;
  int max_ncs, max_id, gmax_id;
  int *idmap;
  int match, found;
  int i, j;

#if 0

  callgraph_to_cstable(ipm_callgraph, &local);
  max_id = callgraph_maxid(ipm_callgraph)+1;
  //print_cstable(stderr, &local);    
 
  IPM_REDUCE( &local.ncs, &max_ncs, 1, MPI_INT, MPI_SUM, 0, MPI_COMM_WORLD);
  IPM_ALLREDUCE( &max_id, &gmax_id, 1, MPI_INT, MPI_MAX, MPI_COMM_WORLD);
  idmap = (int*)IPM_CALLOC(gmax_id, sizeof(int));

  if( task.taskid==0 ) {
    FILE *mapfile;
    char fname[MAXSIZE_FILENAME];
    
    sprintf(fname, "%s.map.txt", task.fname);
    mapfile=fopen(fname, "w");

    init_cstable(&merged, max_ncs);
    init_cstable(&remote, max_ncs);
    /* copy local into merged */
    merge_cstables(&merged, &local, idmap);
    
    for( i=1; i<task.ntasks; i++ )
      {
	clear_cstable(&remote);
	
	IPM_RECV(remote.csids, merged.ncs, MPI_INT, i, 0, MPI_COMM_WORLD, &status); 
	PMPI_Get_count( &status, MPI_INT, &(remote.ncs) );
	IPM_RECV(remote.cstable, remote.ncs*MAXSIZE_CALLSTACKDEPTH*sizeof(void*), 
		  MPI_BYTE, i, 0, MPI_COMM_WORLD, &status );
	
	/* clear ids, merge remote and send map back  */
	for( j=0; j<gmax_id; j++ ) idmap[j]=0;
	merge_cstables(&merged, &remote, idmap);
	IPM_SEND(idmap, gmax_id, MPI_INT, i, 0, MPI_COMM_WORLD);

	
	for( j=0; j<gmax_id; j++ ) {
	  if( idmap[j] && idmap[j]!=j ) {
	    fprintf(mapfile, "rank %d: %u -> %u\n", i, j, idmap[j]);
	  }
	}
      }
    
    fclose(mapfile);
  } 
  else 
    {
      IPM_SEND(local.csids, local.ncs, MPI_INT, 0, 0, MPI_COMM_WORLD );
      IPM_SEND(local.cstable, local.ncs*MAXSIZE_CALLSTACKDEPTH*sizeof(void*),
		MPI_BYTE, 0, 0, MPI_COMM_WORLD);

      for( j=0; j<gmax_id; j++ ) idmap[j]=0;      
      IPM_RECV(idmap, gmax_id, MPI_INT, 0, 0, MPI_COMM_WORLD, &status);
      remap_callsites( ipm_htable, idmap, gmax_id);
    }
#endif
}

#endif  /* HAVE_MPI */

