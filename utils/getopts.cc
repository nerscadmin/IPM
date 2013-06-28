
#include <stdio.h>
#include <stdlib.h>

#include "ipm_parse.h"


int parse_topospec(char* str, job_t *job);

int getopts(int argc, char *argv[], job_t *job)
{
  int i;
  
  job->infile=job->outfile=0;
  job->inname=job->outname="";

  for( i=1; i<argc; i++ ) {
    if( argv[i][0]=='-' ) {
      switch( argv[i][1] )
	{
	case 't': // specify topology 
	  i++;
	  if( i>=argc || !(argv[i]) || 
	      (parse_topospec(argv[i],job)!=IPMP_OK) )
	    {
	      fprintf(stderr, "Error parsing topology specification: '%s'\n",
		      (i>=argc||!(argv[i])?"":argv[i]));
	      return IPMP_ERR;
	    }
	  break;

	case 'f': // full banner
	  if( strcmp(argv[i], "-full") )
	    goto unrecognized;

	  job->outform = FULL;
	  break;

	case 'h': // html report
	  if( strcmp(argv[i], "-html") )
	    goto unrecognized;

	  job->outform = HTML;
	  break;

	case 's': // cube format
	  if( strcmp(argv[i], "-summary") )
	    goto unrecognized;

	  job->outform = SUMMARY;
	  break;
	  
	case 'c': // cube format
	  if( strcmp(argv[i], "-cube") )
	    goto unrecognized;

	  job->outform = CUBE;
	  break;
	  
	case 'o':
	  job->outname=job->inname;
	  job->outname.append(".cube");
	  break;

	case 'q':
	  job->quiet=true;
	  break;
	  
	unrecognized:
	default:
	  fprintf(stderr, "Unrecognized option: '%s'\n", argv[i]);
	  return IPMP_ERR;
	}
    }
    else {
      if( (job->inname)=="" ) {
	job->inname=argv[i];
	continue;
      }
      
      if( (job->outname)=="" ) {
	job->outname=argv[i];
	continue;
      }
      return IPMP_ERR;
    }
  }
  return IPMP_OK;
}


int parse_topospec(char* str, job_t *job)
{
  topospec_t t;
  char *s;

  s=str;
  while( s && (*s) ) {
    t.x=t.y=0; t.z=1;
    
    t.x=strtol(s, &s, 10);
    if( !t.x || (*s!='x') )
      goto error;
    
    t.y=strtol(++s, &s, 10);
    if( !t.y ) goto error;
    
    if( *s=='x' ) {
      t.z=strtol(++s, &s, 10);
      if( !t.z ) goto error;
    }
    
    if( t.x>0 && t.y>0 ) {
      job->topologies.push_back(t);
    }
    
    if( *s==',' ) ++s;
  }
  return IPMP_OK;
  
 error:
  return IPMP_ERR;
}
