
#include <stdlib.h>
#include <string.h>

#include "ipm.h"
#include "ipm_core.h"

#ifdef HAVE_PAPI
#include "mod_papi.h"
#endif


extern char **environ;


#define ENV_DEBUG          0
#define ENV_REPORT         1
#define ENV_LOG            2
#define ENV_LOGDIR         3
#define ENV_HPM            4
#define ENV_OUTFILE        5
#define ENV_LOGWRITER      6
#define ENV_HPCNAME        7
#ifdef HAVE_SNAP
#define ENV_SNAP           8
#endif
#define ENV_NESTED_REGIONS 9


#define MAXSIZE_ENVKEY  120

int ipm_check_env(int env, char*val);


int ipm_tokenize(const char *str, char *key, char *val) {
  char *eqs;
  int len;
  
  key[0]='\0';
  val[0]='\0';

  eqs=strchr(str, '=');
  if(!eqs) {
    key=strncpy(key, str, MAXSIZE_ENVKEY);
    key[MAXSIZE_ENVKEY-1]='\0';
  } else {
    len=eqs-str;
    if( len>MAXSIZE_ENVKEY ) 
      len=MAXSIZE_ENVKEY;
    strncpy(key, str, len);
    key[len]='\0';
    
    strncpy(val, (eqs+1), MAXSIZE_ENVKEY);
    val[MAXSIZE_ENVKEY-1]='\0';
  }
  
  return IPM_OK;
}


int ipm_get_env() 
{
  char *str, **cp;
  int len;
  char key[MAXSIZE_ENVKEY];
  char val[MAXSIZE_ENVKEY];

  ipm_check_env(ENV_DEBUG, getenv("IPM_DEBUG"));

/* defaults for unset environment variables */

  if(!getenv("IPM_HPM")) {
   putenv("IPM_HPM=PAPI_FP_OPS");
  }
  
/* end defaults */

  cp=environ;
  while(str=(*cp)) {
    if( strncmp(str,"IPM", 3)!=0 ) {
      cp++;
      continue;
    }

    ipm_tokenize(str,key,val);
    len = strlen(key);

    /* IPM_DEBUG */
    if(!strcmp("IPM_DEBUG", key)) {
      ipm_check_env(ENV_DEBUG, val);
    } 

   /* IPM_REPORT none|terse|full */
    else if(!strcmp("IPM_REPORT", key)) {
      ipm_check_env(ENV_REPORT, val);
    }
    
    /* IPM_LOG none|terse|full */
    else if(!strcmp("IPM_LOG", key)) {
      ipm_check_env(ENV_LOG, val);
    }

    /* write logs to this directory */
    else if(!strcmp("IPM_LOGDIR", key)) {
      ipm_check_env(ENV_LOGDIR, val);
    }

    /* write log to this file */
    else if(!strcmp("IPM_OUTFILE", key)) {
      ipm_check_env(ENV_OUTFILE, val);
    }

    /* hpcname is a name for the whole compute resource */
    else if(!strcmp("IPM_HPCNAME", key)) {
      ipm_check_env(ENV_HPCNAME, val);
    }

    /* hardware counter settings */
    else if(!strcmp("IPM_HPM", key)) {
      ipm_check_env(ENV_HPM, val);
    }

#ifdef HAVE_SNAP
    else if(!strcmp("IPM_SNAP", key)) {
      ipm_check_env(ENV_SNAP, val);
    }
#endif

    else if(!strcmp("IPM_LOGWRITER", key)) {
      ipm_check_env(ENV_LOGWRITER, val);
    }

    else if(!strcmp("IPM_NESTED_REGIONS", key)) {
      ipm_check_env(ENV_NESTED_REGIONS, val);
    }

    /* do not complain about these */
    else if(!strcmp("IPM_GNU", key)) { }
    else if(!strcmp("IPM_KEYFILE", key)) { }
    else if(!strcmp("IPM_HOME", key)) { }
    else if(!strcmp("IPM_TARGET", key)) { }
    else if(!strcmp("IPM_POST_LINK_OPTS", key)) { }
    else {
/*      IPMERR("Unrecognized env variable %s=%s\n",
	      key, val);
*/
    }
    
    cp++;
  }
  
  return IPM_OK;
}

    
int ipm_check_env(int env, char *val)
{
  switch(env) 
    {
    case ENV_DEBUG:
      if(val) {
	if(val[0] == '*') {
	  task.flags |= FLAG_DEBUG;
	} else {
	  if(atoi(val) == task.taskid) {
	    task.flags |= FLAG_DEBUG;
	  }
	}
      }
     
      break;

    case ENV_REPORT:
      if(!strncmp(val,"none",4) || !strncmp(val,"NONE",4)) {
	FLAG_CLEAR_REPORT(task.flags);
	task.flags |= FLAG_REPORT_NONE;
      }
      else if(!strncmp(val,"terse",5) || !strncmp(val,"TERSE",5) ) { 
	FLAG_CLEAR_REPORT(task.flags);
	task.flags |= FLAG_REPORT_TERSE;
      }
      else if(!strncmp(val,"full",4) || !strncmp(val,"FULL",4) ) {
	FLAG_CLEAR_REPORT(task.flags);
	task.flags |= FLAG_REPORT_FULL;
      }
      else {
	IPMERR("Unrecognized value for IPM_REPORT '%s', ignoring\n", val);
      }
      break;

    case ENV_LOG:
      if(!strncmp(val,"none",4) || !strncmp(val,"NONE",4)) {
	FLAG_CLEAR_LOG(task.flags);
	task.flags |= FLAG_LOG_NONE;
      }
      else if(!strncmp(val,"terse",5) || !strncmp(val,"TERSE",5) ) { 
	FLAG_CLEAR_LOG(task.flags);
	task.flags |= FLAG_LOG_TERSE;
      }
      else if(!strncmp(val,"full",4) || !strncmp(val,"FULL",4) ) {
	FLAG_CLEAR_LOG(task.flags);
	task.flags |= FLAG_LOG_FULL;
      }
      else {
	IPMERR("Unrecognized value for IPM_LOG '%s', ignoring\n", val);
      }
      break;

    case ENV_LOGDIR:
      strcpy(task.logdir, val);
      break;

    case ENV_OUTFILE:
      task.flags |= FLAG_OUTFILE;
      strcpy(task.fname, val);
      break;

    case ENV_HPCNAME:
      task.flags |= FLAG_HPCNAME;
      strncpy(task.hostname, val,MAXSIZE_HOSTNAME);
      break;

    case ENV_LOGWRITER:
      if(!strncmp(val,"serial",6) || !strncmp(val,"SERIAL",6)) {
	FLAG_CLEAR_LOGWRITER(task.flags);
	task.flags |= FLAG_LOGWRITER_POSIXIO;
      }
      else if(!strncmp(val,"parallel",8) || !strncmp(val,"PARALLEL",8) ) {
	FLAG_CLEAR_LOGWRITER(task.flags);
	task.flags |= FLAG_LOGWRITER_MPIIO;
      }
      else {
	IPMERR("Unrecognized value for IPM_LOGWRITER '%s', ignoring\n", val);
      }

      break;


#ifdef HAVE_PAPI
      int i;
      char *uptr;
      char *cptr1;
    case ENV_HPM:
      if (strchr(val,'_')){
	i=0;
	/* User defined list of counters */
	cptr1 = strtok_r(val,",",&uptr);
	while (cptr1){
	  strcpy(papi_events[i].name, cptr1);
	  cptr1 = strtok_r(NULL,",",&uptr);
	  i++;
	}
      } else {
	/* An event set */
      }
      break;
#endif

#ifdef HAVE_SNAP
    case ENV_SNAP: 
      task.snap_period = atof(val);
     break;
#endif

    case ENV_NESTED_REGIONS:
      task.flags |= FLAG_NESTED_REGIONS;
      break;

    default:
      ;
    }
  

  return IPM_OK;
}
