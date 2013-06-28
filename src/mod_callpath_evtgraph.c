

#include <stdio.h>

#include "mod_callpath.h"

int thisblob;


void* cgfunc_markblob(callsite_t *site, int level, int flags, void *ptr) 
{
  int i;
  unsigned csite;
  graph_t *eg;
  node_t *egnode;

  eg = (graph_t*)ptr;

  if( flags==VISIT_BACKTRACK || !(IS_NODE(site)) )
    return ptr;

  /* iterate over all nodes in the graph,
     check if their callsite id equals site->id
     if yes, set the node's tag to thisblob */
  for( i=0; i<eg->nnodes; i++ ) {
    egnode = &(eg->nodes[i]);
    csite = KEY_GET_CALLSITE(egnode->key);
    
    if( csite==(site->id) ) {
      egnode->tag=thisblob;
    }
  }

  return ptr;
}


void* cgfunc_evtgraph(callsite_t *site, int level, int flags, void *ptr) 
{
  int i; char buf[200];
  callsite_t *cgnode;
  unsigned csite;
  node_t *egnode;
  graph_t *eg;
  int fmt;
  FILE *f;

  if( flags==VISIT_BACKTRACK || !(IS_NODE(site)) || IS_LEAF(site) )
    return ptr;
  
  eg = (graph_t*)ptr;

  /* reset tags for whole graph */
  for( i=0; i<eg->nnodes; i++ ) {
    egnode = &(eg->nodes[i]);
    egnode->tag=-1;
    
    /*
    csite = KEY_GET_CALLSITE(egnode->key);
    if( csite==(site->id) ) 
      egnode->tag=-1;
    */
  }
  
  cgnode=site->child; 
  while(cgnode) {
    thisblob=cgnode->id;

    if( IS_LEAF(cgnode) ) {
      for( i=0; i<eg->nnodes; i++ ) {
	egnode = &(eg->nodes[i]);
	csite = KEY_GET_CALLSITE(egnode->key);
	if( csite==(cgnode->id) ) 
	  egnode->tag=0;
      }
    }
    else {
      callgraph_traverse(cgnode, cgnode->parent, cgfunc_markblob, eg);
    }
    
    cgnode=cgnode->next;
  }

  fmt = 
    NODEFMT_COUNT | 
    NODEFMT_CALL |
    NODEFMT_CALLSITE |
    NODEFMT_REGION |
    NODEFMT_RANK |
    NODEFMT_BYTES;

  sprintf(buf, "evtgraph/evtgraph.%d.txt", site->id);
  f=fopen(buf, "w");
  graph_printf(f, eg, fmt);
  fclose(f);

  return ptr;
}


void callgraph_print_evtgraphs(callgraph_t *cg)
{
  graph_t eg;

  htable_to_graph( ipm_htable, MAXSIZE_HASH, &eg );
  callgraph_traverse(cg, 0, cgfunc_evtgraph, &eg);
  graph_free(&eg);
}
