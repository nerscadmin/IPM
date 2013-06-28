
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "ipm.h"
#include "ipm_core.h"
#include "ipm_modules.h"

#include "mod_selfmonitor.h"


#ifdef HAVE_LIBUNWIND
#define UNW_LOCAL_ONLY
#include <libunwind.h>
#endif

#include <mod_callpath.h>

/* root node of the callgraph */
callgraph_t *ipm_callgraph;

/* next id assigned to a node in the calltree */
static unsigned cg_next_id;

static void* cg_stack[MAXSIZE_CALLSTACKDEPTH];

cs_hent_t cs_hash[MAXNUM_CALLSITES];

/* TODO: make output function for this 
#ifdef HAVE_CALLPATH
  system("mkdir -p evtgraph/");
  if(task.ntasks > 1) {
   sprintf(buf, "evtgraph/callgraph.%d.html", task.taskid);
  } else {
   sprintf(buf, "evtgraph/callgraph.html");
  }
  f=fopen(buf, "w");
  callgraph_print_html(f, ipm_callgraph);
  fclose(f);

  callgraph_print_evtgraphs(ipm_callgraph);
#endif
  return 0;
*/

int mod_callpath_init(ipm_mod_t *mod, int flags)
{
  int i;

  mod->state    = STATE_IN_INIT;
  mod->init     = mod_callpath_init;
  mod->output   = 0;
  mod->finalize = 0;
  mod->name     = "CALLPATH";

  cg_next_id=1;
  
  /* make and init root node, id is only assigned when 
     we actually grab the node */
  ipm_callgraph = (callgraph_t*)IPM_MALLOC(sizeof(callsite_t));
  INIT_CALLSITE(ipm_callgraph);

  for(i=0; i<MAXNUM_CALLSITES; i++ ) {
    cs_hash[i].addr=0;
    cs_hash[i].fname=0;
    cs_hash[i].offs=0;
    cs_hash[i].csid=0;
  }

  mod->state    = STATE_ACTIVE;
  return IPM_OK;
}


int cg_stack_adjust(int depth, int a)
{
  int i;
  for( i=0; i<=depth-a; i++ ) {
    cg_stack[i]=cg_stack[i+a];
    }
  for( i=depth-(a-1); i<=depth; i++ ) {
    cg_stack[i]=0;
  }
  return depth-a;
}


#ifdef HAVE_LIBUNWIND 
int getstack_libunwind()
{
  int i, depth;
  unw_cursor_t  cursor;
  unw_context_t context;  
  unw_word_t    offset, pc;
  char          fname[64];
  
  for( i=0; i<MAXSIZE_CALLSTACKDEPTH; i++ )
    cg_stack[i]=0;
  
  unw_getcontext(&context);
  unw_init_local(&cursor, &context);

  depth=0;
  while( unw_step(&cursor)>0 ) 
    {
      unw_get_reg(&cursor, UNW_REG_IP, &pc);

      i=cs_hash_lookup_addr((void*)pc);
      if( cs_hash[i].fname==0 ) {
	fname[0] = '\0';
	(void) unw_get_proc_name(&cursor, fname, sizeof(fname), &offset);
	cs_hash[i].addr=(void*)pc;
	cs_hash[i].offs=offset;
	cs_hash[i].fname=strdup(fname);
      }
      cg_stack[depth++]=(void*)pc;
    }
#ifdef LINUX_XT5
  cg_stack[depth++]=(void*)(-1);
#endif
  depth--;

  /* cg_stack[0]...cg_stack[depth] are filled in */
  
  /* adjust: shift down by two:
     cg_stack[0] is calliste of getstack_libunwind() in get_callsite_id()
     cg_stack[1] is callsite of get_callsite_id() in wrapper function 
  */
  depth = cg_stack_adjust(depth, 2);

  return depth;
}
#endif /* HAVE_LIBUNWIND */


#ifdef HAVE_BACKTRACE
int getstack_backtrace()
{
  int i, depth;

  for( i=0; i<MAXSIZE_CALLSTACKDEPTH; i++ )
    cg_stack[i]=0;

  depth=backtrace(cg_stack, MAXSIZE_CALLSTACKDEPTH);
  depth--;

  /* cg_stack[0]...cg_stack[depth] are filled in */
  
  /* adjust: shift down by three:
     cg_stack[0] is calliste of backtrace() in getstack_backtrace()
     cg_stack[1] is calliste of getstack_backtrace() in get_callsite_id()
     cg_stack[2] is callsite of get_callsite_id() in wrapper function 
  */
  depth = cg_stack_adjust(depth, 3);

  return depth;
}
#endif /* HAVE_BACKTRACE */


int get_callsite_id()
{
  int i, depth;
  unsigned long *ebp;
  callsite_t *site;
  char **strings;

  
  /* retrieve the call stack */
  depth=0;
#ifdef HAVE_LIBUNWIND
  depth=getstack_libunwind();
#endif
#ifdef HAVE_BACKTRACE
  depth=getstack_backtrace();  
#endif

  /*
   * we want to place cg_stack[depth] on the current level in the 
   * tree. site is the 'leftmost' node on the current level 
   */
  site = ipm_callgraph;
  while( depth>=0 )
    {
      if( !(site->addr) ) {
	/* empty node, take it */
	site->addr=cg_stack[depth];
	site->id=cg_next_id++;

	i=cs_hash_lookup_addr((void*)(site->addr));
	if( cs_hash[i].fname!=0  ) {
	  site->name=cs_hash[i].fname;
	  cs_hash[i].csid=site->id;

	  if(site->parent) {
	    cs_hash[i].pcsid=(site->parent)->id;

	    /*
	    fprintf(stderr, "assigned 0x%x %s %d %d %d\n", 
		    cs_hash[i].addr, cs_hash[i].fname, i, 
		    cs_hash[i].csid, cs_hash[i].pcsid);
	    */
	  }
	}
	
	  

	/*
	  fprintf(stderr, "+++ making new node id=%d %p\n", site->id, site->addr);
	*/

	/* make a new, empty child node	*/
	site->child=(callsite_t*)IPM_MALLOC(sizeof(callsite_t));
	INIT_CALLSITE(site->child);	    
	(site->child)->parent=site;
	
	site=site->child;
	depth--;
	continue; 
      }
      if( site->addr==cg_stack[depth] ) {
	/* move to child level */

	/* 
	   fprintf(stderr, "+++ found %p\n", site->addr);
	*/

	if( !(site->child) )
	  {
	    /* make a new, empty child node */
	    site->child=(callsite_t*)IPM_MALLOC(sizeof(callsite_t));
	    INIT_CALLSITE(site->child);	    
	    (site->child)->parent=site;
	  }
	
	site=site->child;
	depth--;
	continue;
      }
      if(!(site->next)) {
	/* 
	   fprintf(stderr, "+++ making next on same level\n");
	*/
	
	/* make next node, take it */
	site->next=(callsite_t*)IPM_MALLOC(sizeof(callsite_t));
	INIT_CALLSITE(site->next);	    
	(site->next)->parent=site->parent;
      }

      /* 
	 fprintf(stderr, "+++ moving to next on same level\n");
      */
	
      site=site->next;
    }
  
  return site->parent->id;
}

int cs_hash_lookup_addr(void *addr) 
{
  unsigned long long i;
  char *ptr;
  int retry;

  i=((unsigned long)addr)%MAXNUM_CALLSITES;

  retry=0;
  do {
    if( (ptr=cs_hash[i].addr) && ptr==addr) {
      return i;
    } else {
      cs_hash[i].addr=addr;
      return i;
    }
    i=(i+1)%MAXNUM_CALLSITES;
  } while(++retry < MAXNUM_CALLSITES);

  return -1;
}

int cs_hash_lookup_csid(int csid) {
  int i;

  for( i=0; i<MAXNUM_CALLSITES; i++ ) {
    if( cs_hash[i].csid==csid )
      return i;
  }
  return -1;
}

int cs_hash_lookup_pcsid(int pcsid) {
  int i;

  for( i=0; i<MAXNUM_CALLSITES; i++ ) {
    if( (cs_hash[i].pcsid)==pcsid )
      return i;
  }
  return -1;
}



#ifdef UTEST_CALLPATH

void fubar() {
  int id;

  id = get_callsite_id();
  fprintf(stderr, "fubar got id %d\n", id);
}

void foo1() {
  int id;

  id = get_callsite_id();
  fprintf(stderr, "foo1 got id %d... ", id);

  fubar();
}

void foo2() {
  int id;

  id = get_callsite_id();
  fprintf(stderr, "foo2 got id %d... ", id);

  fubar();
}

void bar() {
  foo1();
  foo2();
}

int main( int argc, char* argv[])
{
  int i;
  cstable_t cstable;
  cstable_t empty;

  empty.ncs=3;
  empty.csids = IPM_CALLOC(empty.ncs, sizeof(unsigned));
  empty.cstable = IPM_CALLOC(empty.ncs*MAXSIZE_CALLSTACKDEPTH,
			 sizeof(void*));

  //  init_callgraph();
  
  for( i=0; i<3; i++ )
    {
      fprintf(stderr, "iteration %d\n", i+1);
      bar();
      fprintf(stderr, "\n");
    }
  
  /*
  fprintf(stderr, "Printing callgraph (with %u leaves max id %d):\n", 
	  count_leaves(ipm_callgraph), 
	  callgraph_maxid(ipm_callgraph) );
  print_callgraph(ipm_callgraph, stderr);
  fprintf(stderr, "\n");
  */
  
  /*
  callgraph_to_cstable(ipm_callgraph, &cstable);

  print_cstable(stderr, &cstable);

  merge_cstables( &empty, &cstable, 0);
  merge_cstables( &empty, &cstable, 0);

  print_eventgraphs(ipm_callgraph, stderr);
  */
}

#endif
