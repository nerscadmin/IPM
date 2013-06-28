
#include <stdio.h>

#include "mod_selfmonitor.h"
#include "mod_callpath.h"


/*
 * traverse the callgraph and apply 'func' at each node.
 * Stop when backtracking up the parent link would make 'stop'
 * the next node. Setting 'stop' to 0 traverses the entire 
 * callgraph. 'func' is invoked twice for each node: once on the
 * first visit and once while backtracking, 'flags' is set
 * appropriately. 
 */
void callgraph_traverse(callsite_t *g, callsite_t *stop, 
			cgfunc_t func, void *ptr)
{
  int levl;
  callsite_t *node;

  node=g; levl=0;

  while(node) {
 
    /* ----------------------------------- */
    ptr=func(node, levl, VISIT_FIRST, ptr);
    /* ----------------------------------- */

    /* advance to next node (depth-first) */
    if( node->child ) 	{
      levl++;
      node=node->child;
    }
    else if( node->next ) {
      /* levl unchanged! */
      node=node->next;
    }
    else {
      /* back-tracking... */
      while(1) {
	node=node->parent;
	levl--;
	
	/* this will also terminate the outer loop */
	if( !node || node==stop ) 
	  break;

	/* ------------------------------------------- */	
	ptr=func(node, levl, VISIT_BACKTRACK, ptr);
	/* ------------------------------------------- */	

	if( node->next ) {
	  /* levl unchanged! */
	  node=node->next;
	  break;
	}
      }  
      if( node==stop )
	break;
    }
  }
}  



void* cgfunc_count_leaves(callsite_t *site, int level, int flags, void *ptr) 
{
  if( flags==VISIT_BACKTRACK )
    return ptr;
  
  if( IS_NODE(site)&&IS_LEAF(site) )
    (*((int*)ptr))++;
  
  return ptr;
}


void* cgfunc_count_nodes(callsite_t *site, int level, int flags, void *ptr) 
{
  if( flags==VISIT_BACKTRACK )
    return ptr;
  
  if( IS_NODE(site) )
    (*((int*)ptr))++;
  
  return ptr;
}


void* cgfunc_maxid(callsite_t *site, int level, int flags, void *ptr) 
{
  if(flags==VISIT_BACKTRACK) 
    return ptr;

  if( IS_NODE(site) ) {
    if( ((*(unsigned*)ptr))<site->id )
      ((*(unsigned*)ptr))=site->id;
  }
  
  return ptr;
}


void* cgfunc_find_by_csid(callsite_t *site, int level, int flags, void *ptr) 
{
  callsite_t *search = (callsite_t*)ptr;
  
  /* assume search->id specifies the id we are looking for 
     and search->addr==0 initially */
  if(flags==VISIT_BACKTRACK || search->addr ) 
      return ptr;

  if(site->id == search->id ) {
    COPY_CALLSITE(site, search);
  }
  
  return ptr;
}


void* cgfunc_print_csite(callsite_t *site, int level, int flags, void *ptr) 
{
  int i;
  FILE *f = (FILE*)ptr;
  
  if(flags==VISIT_BACKTRACK) 
    return ptr;

  if(IS_NODE(site)) {
    for( i=0; i<level; i++ )
      fprintf(f, "  ");
    fprintf(f, "ID=%u (addr=%p) %s\n", site->id, 
	    site->addr, IS_LEAF(site)?"leaf":"");
  }

  return ptr;
}


void* cgfunc_print_csite_html(callsite_t *site, int level, int flags, void *ptr)  
{
  int i;
  FILE *f = (FILE*)ptr;

  if(!(IS_NODE(site))) return ptr;

  /* deal with backtracking */
  if(flags==VISIT_BACKTRACK) {
    if( !(IS_LEAF(site)) ) {
      for( i=0; i<level; i++ )
        fprintf(f, "  ");
      fprintf(f, "</li>\n");
    }
    
    if(!(site->next) ) {
      for( i=0; i<level; i++ )
        fprintf(f, "  ");
      fprintf(f, "</ul>\n");
    }
    return ptr;
  }
  
  for( i=0; i<level; i++ )
    fprintf(f, "  ");

  if( IS_LEAF(site) ) {
    i=cs_hash_lookup_addr(site->addr);
    fprintf(f, "<li>%s() [%s()+0x%x] &lt;ID %u&gt; </li>\n",
	    /*callsite2callname(site->id)*/"", 
	    cs_hash[i].fname, cs_hash[i].offs, site->id);
    /*
    fprintf(f, "<li>ID=%u addr=%p</li>\n",
	    site->id, site->addr);
    */
  }
  else {
    i=cs_hash_lookup_addr(site->child->addr);

    fprintf(f, "<li><a href=\"evtgraph.%d.svg\" target=\"ifrm\">%s() &lt;ID %u&gt; </a>\n",
	    site->id, cs_hash[i].fname, site->id);
    /*
    fprintf(f, "<li><a href=\"evtgraph.%d.svg\" target=\"ifrm\">ID=%u addr=%p</a>\n",
	    site->id, site->id, site->addr);
    */
  }
  
  if(site->child && (site->child)->addr ) {
    for( i=0; i<level+1; i++ )
      fprintf(f, "  ");
    fprintf(f, "<ul>\n");
  }
  
  return ptr;
}


int callgraph_count_leaves(callgraph_t *g)
{
  int count=0;

  callgraph_traverse(g, 0, cgfunc_count_leaves, &count);
  return count;
}

int callgraph_count_nodes(callgraph_t *g)
{
  int count=0;

  callgraph_traverse(g, 0, cgfunc_count_nodes, &count);
  return count;
}

int callgraph_maxid(callgraph_t *g)
{
  int maxid=0;
  
  callgraph_traverse(g, 0, cgfunc_maxid, &maxid);
  return maxid;
}

void callgraph_print(callgraph_t *g, FILE *f)
{
  callgraph_traverse(g, 0, cgfunc_print_csite, f);
}

void callgraph_print_html(callgraph_t *g, FILE *f)
{
  fprintf(f, "<html><body><ul>\n");
  callgraph_traverse(g, 0, cgfunc_print_csite_html, f);
  fprintf(f, "</ul></body></html>\n");
}

void callgraph_find_by_csid(callgraph_t *g, callsite_t *site)
{
  callgraph_traverse(g, 0, cgfunc_find_by_csid, site);
}


