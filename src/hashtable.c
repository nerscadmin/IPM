#include <stdio.h>
#include <stdlib.h>

#include "hashkey.h"
#include "hashtable.h"
#include "calltable.h"


/* ---- global variables ----- */

ipm_hent_t ipm_htable[MAXSIZE_HASH];
int ipm_hspace; /* space remaining in hash */


void htable_init(ipm_hent_t *table)
{
  int i;
  ipm_hspace=MAXSIZE_HASH;

  for(i=0; i<MAXSIZE_HASH; i++ )  {
    HENT_CLEAR(table[i]);
  }
}

void htable_dump(FILE *f, ipm_hent_t *table, int hdr)
{
  int call, tid, rank, reg, csite;
  unsigned bytes;
  char *activity;

  unsigned i;

  if( hdr ) {
    fprintf(f, "#  index :  call   reg csite  rank   tid     bytes     count   (   tmin,    tmax,    ttot)\n");
  }

  for( i=0; i<MAXSIZE_HASH; i++ )
    {
      if( table[i].count==0 )
        continue;
      
      call  = KEY_GET_ACTIVITY(table[i].key);
      reg   = KEY_GET_REGION(table[i].key);
      csite = KEY_GET_CALLSITE(table[i].key);
      rank  = KEY_GET_RANK(table[i].key);
      tid   = KEY_GET_TID(table[i].key);
      bytes = KEY_GET_BYTES(table[i].key);

      if( call<KEY_MIN_ACTIVITY || call>KEY_MAX_ACTIVITY ||
	  call<0 || call>=MAXSIZE_CALLTABLE ) {
	activity = "(out of bounds)";
      } else {
	activity = ipm_calltable[call].name;
      }
      
      
      fprintf(f, "%8d : %5d %5d %5d %5d %5d %9u %9"IPM_COUNT_TYPEF"   (%7.2g, %7.2g, %7.2g) [%s]\n",
	      i, call, reg, csite, rank, tid, bytes,
	      table[i].count, 
	      table[i].t_min, table[i].t_max, table[i].t_tot,
	      activity);
    }
}


/* 
 * re-map the callsites stored in the hashtable, i.e., replace the
 * callsite CS with map[CS]. This is used in the callsite unification
 * step done to get a consisitent numbering of callsite accross MPI
 * ranks.
 */
void htable_remap_callsites(ipm_hent_t *table, int *map, int maxid)
{
  int i; int cs;
  
  for( i=0; i<MAXSIZE_HASH; i++ ) {
    if( table[i].count==0 )
      continue;
    
    cs=KEY_GET_CALLSITE(table[i].key);
    
    if( 0<=cs && cs<=maxid && map[cs] && cs!=map[cs] ) {
      KEY_SET_CALLSITE(table[i].key,map[cs]);
    }
  }
}



/*
 * clear all entries in the hashtable that match spec 
 */
void htable_clear( ipm_hent_t *table, scanspec_t spec ) 
{ 
  int i;
  unsigned lact, lreg, lcal, lrnk, ltid, lbyt;
  unsigned hact, hreg, hcal, hrnk, htid, hbyt;
  unsigned act, reg, cal, rnk, tid, byt;

  lact = KEY_GET_ACTIVITY(spec.lo);
  lreg = KEY_GET_REGION(spec.lo);
  lcal = KEY_GET_CALLSITE(spec.lo);
  lrnk = KEY_GET_RANK(spec.lo);
  ltid = KEY_GET_TID(spec.lo);
  lbyt = KEY_GET_BYTES(spec.lo);

  hact = KEY_GET_ACTIVITY(spec.hi);
  hreg = KEY_GET_REGION(spec.hi);
  hcal = KEY_GET_CALLSITE(spec.hi);
  hrnk = KEY_GET_RANK(spec.hi);
  htid = KEY_GET_TID(spec.hi);
  hbyt = KEY_GET_BYTES(spec.hi);

  for( i=0; i<MAXSIZE_HASH; i++ )
    {
      if( table[i].count==0 )
        continue;

      act = KEY_GET_ACTIVITY(table[i].key);
      reg = KEY_GET_REGION(table[i].key);
      cal = KEY_GET_CALLSITE(table[i].key);
      rnk = KEY_GET_RANK(table[i].key);
      tid = KEY_GET_TID(table[i].key);
      byt = KEY_GET_BYTES(table[i].key);
      
      if( (lact<=act && act<=hact) &&
	  (lreg<=reg && reg<=hreg) &&
	  (lcal<=cal && cal<=hcal) &&
	  (lrnk<=rnk && rnk<=hrnk) &&
	  (ltid<=tid && tid<=htid) &&
	  (lbyt<=byt && byt<=hbyt) )
	{
	  HENT_CLEAR(table[i]);
	}
    }
}


/*
 * scan the hashtable for entries that match scanspec_t 'spec',
 * put the aggregate statistics for these entries into 'stats',
 * return the number of matching entries
 */
int htable_scan( const ipm_hent_t *table, scanstats_t *stats, 
		 scanspec_t spec )
{
  int res, i;
  unsigned lact, lreg, lcal, lrnk, ltid, lbyt;
  unsigned hact, hreg, hcal, hrnk, htid, hbyt;
  unsigned act, reg, cal, rnk, tid, byt;

  lact = KEY_GET_ACTIVITY(spec.lo);
  lreg = KEY_GET_REGION(spec.lo);
  lcal = KEY_GET_CALLSITE(spec.lo);
  lrnk = KEY_GET_RANK(spec.lo);
  ltid = KEY_GET_TID(spec.lo);
  lbyt = KEY_GET_BYTES(spec.lo);

  hact = KEY_GET_ACTIVITY(spec.hi);
  hreg = KEY_GET_REGION(spec.hi);
  hcal = KEY_GET_CALLSITE(spec.hi);
  hrnk = KEY_GET_RANK(spec.hi);
  htid = KEY_GET_TID(spec.hi);
  hbyt = KEY_GET_BYTES(spec.hi);

  res=0;
  stats->bytesum=0.0;
  stats->hent.count=0;
  stats->hent.t_min=1.0e6; /* FIXME*/
  stats->hent.t_max=0.0;
  stats->hent.t_tot=0.0;

  for( i=0; i<MAXSIZE_HASH; i++ )
    {
      if( table[i].count==0 )
        continue;

      act = KEY_GET_ACTIVITY(table[i].key);
      reg = KEY_GET_REGION(table[i].key);
      cal = KEY_GET_CALLSITE(table[i].key);
      rnk = KEY_GET_RANK(table[i].key);
      tid = KEY_GET_TID(table[i].key);
      byt = KEY_GET_BYTES(table[i].key);
      
      if( (lact<=act && act<=hact) &&
	  (lreg<=reg && reg<=hreg) &&
	  (lcal<=cal && cal<=hcal) &&
	  (lrnk<=rnk && rnk<=hrnk) &&
	  (ltid<=tid && tid<=htid) &&
	  (lbyt<=byt && byt<=hbyt) )
	{
	  res++;
	  stats->hent.count+=table[i].count;
	  stats->hent.t_tot+=table[i].t_tot;

	  stats->bytesum += 
	    (double)KEY_GET_BYTES(table[i].key) *
	    (double)table[i].count;
	  
	  if( table[i].t_min<stats->hent.t_min )
	    stats->hent.t_min=table[i].t_min;
	  if( table[i].t_max>stats->hent.t_max )
	    stats->hent.t_max=table[i].t_max;
	}
    }
  return res;
}

/*
 * scan the hashtable for multiple matches simultaneously 
 * implemented as one pass over the hashtable 
 */
int htable_scan_multi( const ipm_hent_t *table, int nspec, 
		       ipm_hent_t stats[], scanspec_t spec[] ) 
{
  int res, i, j;
  unsigned lact, lreg, lcal, lrnk, ltid, lbyt;
  unsigned hact, hreg, hcal, hrnk, htid, hbyt;
  unsigned act, reg, cal, rnk, tid, byt;
  res=0;
  for( i=0; i<MAXSIZE_HASH; i++ )
    {
      if( table[i].count==0 )
        continue;
      
      act = KEY_GET_ACTIVITY(table[i].key);
      reg = KEY_GET_REGION(table[i].key);
      cal = KEY_GET_CALLSITE(table[i].key);
      rnk = KEY_GET_RANK(table[i].key);
      tid = KEY_GET_TID(table[i].key);
      byt = KEY_GET_BYTES(table[i].key);

      for( j=0; j<nspec; j++ ) 
	{
	  lact = KEY_GET_ACTIVITY(spec[j].lo);
	  lreg = KEY_GET_REGION(spec[j].lo);
	  lcal = KEY_GET_CALLSITE(spec[j].lo);
	  lrnk = KEY_GET_RANK(spec[j].lo);
	  ltid = KEY_GET_TID(spec[j].lo);
	  lbyt = KEY_GET_BYTES(spec[j].lo);
	  
	  hact = KEY_GET_ACTIVITY(spec[j].hi);
	  hreg = KEY_GET_REGION(spec[j].hi);
	  hcal = KEY_GET_CALLSITE(spec[j].hi);
	  hrnk = KEY_GET_RANK(spec[j].hi);
	  htid = KEY_GET_TID(spec[j].hi);
	  hbyt = KEY_GET_BYTES(spec[j].hi);
	
	  if( (lact<=act && act<=hact) &&
	      (lreg<=reg && reg<=hreg) &&
	      (lcal<=cal && cal<=hcal) &&
	      (lrnk<=rnk && rnk<=hrnk) &&
	      (ltid<=tid && tid<=htid) &&
	      (lbyt<=byt && byt<=hbyt) )
	    {
	      res++;
	      stats[j].count+=table[i].count;
	      stats[j].t_tot+=table[i].t_tot;
	      
	      if( table[i].t_min<stats[j].t_min )
		stats[j].t_min=table[i].t_min;
	      if( table[i].t_max>stats[j].t_max )
		stats[j].t_max=table[i].t_max;
	    }	
	}
    }

  return res;
}


/*
 * scan the full hashtable and provide the statistics in 
 * stats. This assumes that stats is big enough to hold 
 * data from any activity contained in the hashtable, i.e., 
 * stats[act] is always well defined.
 */
int htable_scan_full( const ipm_hent_t *table, 
		      ipm_hent_t stats[], scanspec_t spec)
{
  int res, i;
  unsigned lact, lreg, lcal, lrnk, ltid, lbyt;
  unsigned hact, hreg, hcal, hrnk, htid, hbyt;
  unsigned act, reg, cal, rnk, tid, byt;

  lact = KEY_GET_ACTIVITY(spec.lo);
  lreg = KEY_GET_REGION(spec.lo);
  lcal = KEY_GET_CALLSITE(spec.lo);
  lrnk = KEY_GET_RANK(spec.lo);
  ltid = KEY_GET_TID(spec.lo);
  lbyt = KEY_GET_BYTES(spec.lo);

  hact = KEY_GET_ACTIVITY(spec.hi);
  hreg = KEY_GET_REGION(spec.hi);
  hcal = KEY_GET_CALLSITE(spec.hi);
  hrnk = KEY_GET_RANK(spec.hi);
  htid = KEY_GET_TID(spec.hi);
  hbyt = KEY_GET_BYTES(spec.hi);

  res=0;
  for( i=0; i<MAXSIZE_HASH; i++ )
    {
      if( table[i].count==0 )
        continue;
      res++;

      act = KEY_GET_ACTIVITY(table[i].key);
      reg = KEY_GET_REGION(table[i].key);
      cal = KEY_GET_CALLSITE(table[i].key);
      rnk = KEY_GET_RANK(table[i].key);
      tid = KEY_GET_TID(table[i].key);
      byt = KEY_GET_BYTES(table[i].key);

      if( (lact<=act && act<=hact) &&
	  (lreg<=reg && reg<=hreg) &&
	  (lcal<=cal && cal<=hcal) &&
	  (lrnk<=rnk && rnk<=hrnk) &&
	  (ltid<=tid && tid<=htid) &&
	  (lbyt<=byt && byt<=hbyt) )
	{
	  stats[act].count+=table[i].count;
	  stats[act].t_tot+=table[i].t_tot;
	  
	  if( table[i].t_min<stats[act].t_min )
	    stats[act].t_min=table[i].t_min;
	  if( table[i].t_max>stats[act].t_max )
	    stats[act].t_max=table[i].t_max;
	}
    }

  return res;
}


int htable_scan_activity(const ipm_hent_t *table, scanstats_t *stats,
			 unsigned beg, unsigned end )
{
  int res;
  scanspec_t spec;
  
  scanspec_unrestrict_all(&spec);
  scanspec_restrict_activity(&spec, beg, end);
  
  res = htable_scan( table, stats, spec);
  return res;
}

void scanspec_unrestrict_all(scanspec_t *spec) 
{
  KEY_CLEAR(spec->lo);
  KEY_CLEAR(spec->hi);
  
  KEY_SET_ACTIVITY(spec->lo, KEY_MIN_ACTIVITY);
  KEY_SET_REGION(spec->lo, KEY_MIN_REGION);
  KEY_SET_CALLSITE(spec->lo, KEY_MIN_CALLSITE);
  KEY_SET_RANK(spec->lo, KEY_MIN_RANK);
  KEY_SET_TID(spec->lo, KEY_MIN_TID);
  KEY_SET_BYTES(spec->lo, KEY_MIN_BYTES);
  
  KEY_SET_ACTIVITY(spec->hi, KEY_MAX_ACTIVITY);
  KEY_SET_REGION(spec->hi, KEY_MAX_REGION);
  KEY_SET_CALLSITE(spec->hi, KEY_MAX_CALLSITE);
  KEY_SET_RANK(spec->hi, KEY_MAX_RANK);
  KEY_SET_TID(spec->hi, KEY_MAX_TID);
  KEY_SET_BYTES(spec->hi, KEY_MAX_BYTES);
}

void scanspec_null(scanspec_t *spec) 
{
  KEY_CLEAR(spec->lo);
  KEY_CLEAR(spec->hi);

  KEY_SET_ACTIVITY(spec->hi, KEY_MIN_ACTIVITY);
  KEY_SET_REGION(spec->hi, KEY_MIN_REGION);
  KEY_SET_CALLSITE(spec->hi, KEY_MIN_CALLSITE);
  KEY_SET_RANK(spec->hi, KEY_MIN_RANK);
  KEY_SET_TID(spec->hi, KEY_MIN_TID);
  KEY_SET_BYTES(spec->hi, KEY_MIN_BYTES);
  
  KEY_SET_ACTIVITY(spec->lo, KEY_MAX_ACTIVITY);
  KEY_SET_REGION(spec->lo, KEY_MAX_REGION);
  KEY_SET_CALLSITE(spec->lo, KEY_MAX_CALLSITE);
  KEY_SET_RANK(spec->lo, KEY_MAX_RANK);
  KEY_SET_TID(spec->lo, KEY_MAX_TID);
  KEY_SET_BYTES(spec->lo, KEY_MAX_BYTES);
}


/* --- restrict --- */

void scanspec_restrict_activity(scanspec_t *spec, 
				unsigned beg, unsigned end ) 
{
  KEY_SET_ACTIVITY(spec->lo, beg);
  KEY_SET_ACTIVITY(spec->hi, end);
}

void scanspec_restrict_region(scanspec_t *spec, 
				unsigned beg, unsigned end ) 
{
  KEY_SET_REGION(spec->lo, beg);
  KEY_SET_REGION(spec->hi, end);
}

void scanspec_restrict_callsite(scanspec_t *spec, 
				  unsigned beg, unsigned end ) 
{
  KEY_SET_CALLSITE(spec->lo, beg);
  KEY_SET_CALLSITE(spec->hi, end);
}

void scanspec_restrict_rank(scanspec_t *spec, 
			      unsigned beg, unsigned end ) 
{
  KEY_SET_RANK(spec->lo, beg);
  KEY_SET_RANK(spec->hi, end);
}

void scanspec_restrict_tid(scanspec_t *spec, 
			     unsigned beg, unsigned end ) 
{
  KEY_SET_TID(spec->lo, beg);
  KEY_SET_TID(spec->hi, end);
}

void scanspec_restrict_bytes(scanspec_t *spec, 
			       unsigned beg, unsigned end ) 
{
  KEY_SET_BYTES(spec->lo, beg);
  KEY_SET_BYTES(spec->hi, end);
}

/* --- unrestrict --- */

void scanspec_unrestrict_activity(scanspec_t *spec)
{
  KEY_SET_ACTIVITY(spec->lo, KEY_MIN_ACTIVITY);
  KEY_SET_ACTIVITY(spec->hi, KEY_MAX_ACTIVITY);
}

void scanspec_unrestrict_region(scanspec_t *spec, 
			      unsigned beg, unsigned end ) 
{
  KEY_SET_REGION(spec->lo, KEY_MIN_REGION);
  KEY_SET_REGION(spec->hi, KEY_MAX_REGION);
}

void scanspec_unrestrict_callsite(scanspec_t *spec, 
				unsigned beg, unsigned end ) 
{
  KEY_SET_CALLSITE(spec->lo, KEY_MIN_CALLSITE);
  KEY_SET_CALLSITE(spec->hi, KEY_MAX_CALLSITE);
}

void scanspec_unrestrict_rank(scanspec_t *spec, 
			    unsigned beg, unsigned end ) 
{
  KEY_SET_RANK(spec->lo, KEY_MIN_RANK);
  KEY_SET_RANK(spec->hi, KEY_MAX_RANK);
}

void scanspec_unrestrict_tid(scanspec_t *spec, 
			   unsigned beg, unsigned end ) 
{
  KEY_SET_TID(spec->lo, KEY_MIN_TID);
  KEY_SET_TID(spec->hi, KEY_MAX_TID);
}

void scanspec_unrestrict_bytes(scanspec_t *spec, 
			   unsigned beg, unsigned end ) 
{
  KEY_SET_BYTES(spec->lo, KEY_MIN_BYTES);
  KEY_SET_BYTES(spec->hi, KEY_MAX_BYTES);
}





#ifdef UTEST_HASHTABLE

int main(int argc, char* argv[])
{
  int i1, i2;
  ipm_hent_t t[MAXSIZE_HASH];
  IPM_KEY_TYPE key;

  fprintf(stderr, "Initializing hashtable... ");
  htable_init(t);

  fprintf(stderr, "size: %u bytes, %d entries\n", 
	  sizeof(t), MAXSIZE_HASH);

  fprintf(stderr, "Dumping table (should be empty)...");
  htable_dump(stderr, t, 0);
  fprintf(stderr, " done.\n");

  KEY_CLEAR(key);
  KEY_SET_ACTIVITY(key, 1);
  KEY_SET_REGION(key, 22);
  KEY_SET_BYTES(key, 2048);
  KEY_SET_RANK(key, 3);
  KEY_SET_CALLSITE(key, 5);
  KEY_SET_TID(key, 0);

  IPM_HASH_HKEY(t, key, i1);
  t[i1].count++;
  t[i1].t_min=0.11;
  t[i1].t_max=0.11;
  t[i1].t_tot=0.11;
   
  fprintf(stderr, "Dumping table:\n");
  htable_dump(stderr, t, 1);
}

#endif /* UTEST_HASHTABLE */



#ifdef UTEST_HASHPERF

#include "../include/ipm_time.h"

typedef struct evtdata
{
  IPM_KEY_TYPE sig;
  double time;
} evtdata_t;

int main(int argc, char* argv[])
{
  int i, idx;
  unsigned coll;
  IPM_KEY_TYPE key;
  ipm_hent_t table[MAXSIZE_HASH];
  double t1, t2, v;

  int nsig; /* number of signatures (unique events) */
  int nevent;
  evtdata_t *evtdata;

  for( i=0; i<MAXSIZE_CALLTABLE; i++ ) {
    ipm_calltable[i].name = (char*)malloc(16);
    sprintf(ipm_calltable[i].name, "Call [%d]", i);
  }
  
  nevent = 10000000;

  for( nsig=1000; nsig<=32000; nsig+=1000 ) 
    {
      evtdata = (evtdata_t*) malloc( sizeof(evtdata_t)*nsig);
      
      for( i=0; i<nsig; i++ ) {
	KEY_RANDOM( evtdata[i].sig ) ;
	evtdata[i].time = drand48();
      }

      htable_init(table);
      coll=0;

      IPM_TIMESTAMP(t1);
      for( i=0; i<nevent; i++ ) {
	IPM_HASH_HKEY_COLL(table, evtdata[i%nsig].sig, idx, coll);
	if( idx>=0 ) {
	  v= evtdata[i%nsig].time;
	  table[idx].count++;
	  table[idx].t_tot+=v;
	  if( v<table[idx].t_min ) table[idx].t_min=v;
	  if( v>table[idx].t_max ) table[idx].t_max=v;
	}
      }
      IPM_TIMESTAMP(t2);
    
      fprintf(stderr, "hashing %d events: (%d unique keys): %f secs (%d collisions)\n", 
	      nevent, nsig, t2-t1, coll);
      
      free(evtdata);
    }
  
  //htable_dump(stderr, table, 1);
}

#endif /* UTEST_HASHPERF */
