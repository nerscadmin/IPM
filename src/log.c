#include <stdio.h>
#include <time.h>

#include "ipm.h"
#include "ipm_types.h"
#include "hashtable.h"
#include "hashkey.h"

#ifdef HAVE_MPI
#include <mpi.h>
#include "GEN.calltable_mpi.h"
#endif
#ifdef HAVE_POSIXIO
#include "GEN.calltable_posixio.h"
#endif
#ifdef HAVE_MEM
#include "GEN.calltable_mem.h"
#endif

int ipm_log(char *op, struct ipm_jobdata *j, struct ipm_taskdata *t, char *literal, void *fh,size_t buff_in,int pflag) { 

#ifdef HAVE_MPI
#define GOT_LOG_METHOD
#endif

 if(strncmp(op,"r",1)==0) { /* read one task struct from a job file and return */
 }

 if(strncmp(op,"w",1)==0) {/* write a log file */
 }

 bsize=0;
/*********************************************/
/* emit a task profile to ipm_mpi_log_fh  */
/*********************************************/
  if(t->mpi_rank==0) {
    rv = ipmprintf(fh,buff_in,pflag,
	 "<?xml version=\"1.0\" encoding=\"iso-8859-1\"?>\n<ipm_job_profile>\n");
    }

   rv = ipmprintf(fh,buff_in,pflag, "<task ipm_version=\"%s\" cookie=\"%s\" mpi_rank=\"%d\" mpi_size=\"%d\" stamp_init=\"%.6f\" stamp_final=\"%.6f\" username=\"%s\" allocationname=\"%s\" flags=\"%lld\" pid=\"%d\" >\n",
	IPM_VERSION,
	j->cookie,
	t->mpi_rank,
	t->mpi_size,
	t->stamp_init,
	t->stamp_final,
	j->username,
	j->allocationname,
	t->flags,
	t->pid);

   rv = ipmprintf(fh,buff_in,pflag,
	 "<job nhosts=\"%d\" ntasks=\"%d\" start=\"%d\" final=\"%d\" cookie=\"%s\" code=\"%s\" >%s</job>\n",
        j->nhosts,
        j->ntasks,
	(int)t->stamp_init,
	(int)t->stamp_final,
	j->cookie,
	j->codename,
	j->id);

   rv = ipmprintf(fh,buff_in,pflag,
	 "<host mach_name=\"%s\" mach_info=\"%s\" >%s</host>\n",
	IPM_HPCNAME,
	t->mach_info,
	t->hostname);
   
   
   if(t->flops == -1.0) {
    b_flops = -1.0;
   } else {
    b_flops = t->flops*OOGU;
   }

   
   rv = ipmprintf(fh,buff_in,pflag,
         "<perf wtime=\"%.5e\" utime=\"%.5e\" stime=\"%.3e\" mtime=\"%.5e\" gflop=\"%.5e\" gbyte=\"%.5e\" ></perf>\n",
	t->wtime,
	t->utime,
	t->stime,
	t->mtime,
	b_flops,
	t->bytes*OOGB);

   rv = ipmprintf(fh,buff_in,pflag,
         "<switch bytes_tx=\"%.5e\" bytes_rx=\"%.5e\" > %s </switch>\n",
	t->switch_bytes_tx,
	t->switch_bytes_rx,
	t->switch_name);
   
   rv = ipmprintf(fh,buff_in,pflag, "<cmdline realpath=\"%s\" >%s</cmdline>\n",
	 t->mpi_realpath,
	 t->mpi_cmdline);

#ifndef IPM_DISABLE_EXECINFO

/* the fgets here should go away FIXME */ 
   rv = ipmprintf(fh,buff_in,pflag, "<exec><pre>\n");
   
   sprintf(cmd, "/usr/bin/file ");
   if(strcmp(t->mpi_realpath,"unknown") &&
      (strlen(cmd)+strlen(t->mpi_realpath)+1)<MAXSIZE_TXTLINE) {
     
     sprintf(cmd, "%s%s\n", cmd, t->mpi_realpath);
     in_fh = popen(cmd,"r"); 
     if(in_fh) {
	while(fgets(cmd,MAXSIZE_TXTLINE,in_fh)) {
	  rv=ipmprintf(fh,buff_in,pflag, "%s", cmd);
	}
	pclose(in_fh);
      }
    } else {
     rv = ipmprintf(fh,buff_in,pflag, "unknown\n");
    }
    rv = ipmprintf(fh,buff_in,pflag, "</pre></exec>\n");
    
    rv = ipmprintf(fh,buff_in,pflag, "<exec_bin><pre>\n");


#ifdef OS_LINUX
    sprintf(cmd, "/usr/bin/ldd ");
#endif
#ifdef OS_AIX
    sprintf(cmd, "/usr/bin/dump -X32_64 -H ");
#endif
    if(strcmp(t->mpi_realpath,"unknown") &&
       (strlen(cmd)+strlen(t->mpi_realpath)+1)<MAXSIZE_TXTLINE) {

     sprintf(cmd, "%s%s\n", cmd, t->mpi_realpath);

     if(strlen(cmd) > 1) {
      in_fh = popen(cmd,"r"); 
      if(in_fh) {
       while(fgets(cmd,MAXSIZE_TXTLINE,in_fh)) {
        rv=ipmprintf(fh,buff_in,pflag, "%s", cmd);
       }
       pclose(in_fh);
      }
     }  else {
      rv = ipmprintf(fh,buff_in,pflag, "unknown\n");
     }
    } else {
     rv = ipmprintf(fh,buff_in,pflag, "unknown\n"); 
    }
    rv = ipmprintf(fh,buff_in,pflag, "</pre></exec_bin>\n");
#endif

   if(strlen(literal) > 0 ) {
      rv = ipmprintf(fh,buff_in,pflag, "%s\n", literal);
   }

   if(1) {
   if(t->mpi_rank == 0) {
    envp = environ;
    while(*envp) {
     if(strstr(*envp,"=")) {
      rv = ipmprintf(fh,buff_in,pflag, "<env>%s</env>\n", *envp);
      envp++;
     } else {
      break;
     }
    }
   }
   }

#define FORMAT_RU(key,u) {\
   rv=ipmprintf(fh,buff_in,pflag,\
	"<%s>%.4e %.4e %lld %lld %lld %lld %lld %lld %lld %lld %lld %lld %lld %lld %lld %lld</%s>\n",\
	key,\
        u.ru_utime.tv_sec + 1.0e-6*u.ru_utime.tv_usec,\
        u.ru_stime.tv_sec + 1.0e-6*u.ru_stime.tv_usec,\
	(long long int)u.ru_maxrss,\
	(long long int)u.ru_ixrss,\
	(long long int)u.ru_idrss,\
	(long long int)u.ru_isrss,\
	(long long int)u.ru_minflt,\
	(long long int)u.ru_majflt,\
	(long long int)u.ru_nswap,\
	(long long int)u.ru_inblock,\
	(long long int)u.ru_oublock,\
	(long long int)u.ru_msgsnd,\
	(long long int)u.ru_msgrcv,\
	(long long int)u.ru_nsignals,\
	(long long int)u.ru_nvcsw,\
	(long long int)u.ru_nivcsw,\
        key);\
}
   FORMAT_RU("ru_s_ti",t->ru_SELF_init);  
   FORMAT_RU("ru_s_tf",t->ru_SELF_final);
   FORMAT_RU("ru_c_ti",t->ru_CHILD_init);
   FORMAT_RU("ru_c_tf",t->ru_CHILD_final);

#undef FORMAT_RU

   
   rv = ipmprintf(fh,buff_in,pflag, "<call_mask >\n");
   for(icall=0;icall<=MAXSIZE_CALLS;icall++) {
    if( (~CALL_BUF_PRECISION(icall) & CALL_MASK_BPREC) ) {
     rv = ipmprintf(fh,buff_in,pflag, "<call prec=\"%lld\" > </call>\n",
	(~CALL_BUF_PRECISION(icall) & CALL_MASK_BPREC));
     
    }
   }
   rv = ipmprintf(fh,buff_in,pflag, "</call_mask >\n");
 
   rv = ipmprintf(fh,buff_in,pflag, "<regions n=\"%d\" >\n",t->region_nregion);
  for(ireg=0;ireg<t->region_nregion;ireg++) {
   rv = ipmprintf(fh,buff_in,pflag,
	 "<region label=\"%s\" nexits=\"%lld\" wtime=\"%.4e\" utime=\"%.4e\" stime=\"%.4e\" mtime=\"%.4e\" >\n ",
	t->region_label[ireg],
	t->region_count[ireg],
	t->region_wtime[ireg],
	t->region_utime[ireg],
	t->region_stime[ireg],
	t->region_mtime[ireg]);  
   rv = ipmprintf(fh,buff_in,pflag,
	"<hpm api=\"%s\" ncounter=\"%d\" eventset=\"%d\" gflop=\"%.4e\" >\n",
#ifdef HPM_PAPI
	"PAPI",
#endif
#ifdef HPM_PMAPI
	"PMAPI",
#endif
#ifdef HPM_DISABLED
	"DISABLED",
#endif
	MAXSIZE_HPMCOUNTERS,
	t->hpm_eventset,
	IPM_HPM_CALC_FLOPS(t->hpm_count[ireg][t->hpm_eventset])*OOGU);
   if(t->flags & IPM_HPM_CANCELED) {
    rv = ipmprintf(fh,buff_in,pflag,"CANCELED");
   } else {

    for(i=0;i<MAXSIZE_HPMCOUNTERS;i++) {
     ii = ipm_hpm_eorder[t->hpm_eventset][i];
     rv = ipmprintf(fh,buff_in,pflag, "<counter name=\"%s\" > %lld </counter>\n",
         ipm_hpm_ename[t->hpm_eventset][ii],
	 t->hpm_count[ireg][t->hpm_eventset][ii]
	);
   }
  }
  rv = ipmprintf(fh,buff_in,pflag, "</hpm>\n");

   for(icall=0;icall<=MAXSIZE_CALLS;icall++) {
    if(t->call_mcount[ireg][icall] > 0.0) {

     rv=ipmprintf(fh,buff_in,pflag,"<func name=\"%s\" count=\"%lld\" > %.4e </func>\n",
	t->call_label[icall],
	t->call_mcount[ireg][icall],
	t->call_mtime[ireg][icall]);
     
    }
   }

   rv = ipmprintf(fh,buff_in,pflag,"</region>\n");
  }
   rv = ipmprintf(fh,buff_in,pflag, "</regions>\n"); 

  if(task.flags & LOG_FULL) {
    rv = ipmprintf(fh,buff_in,pflag,
	"<hash nlog=\"%lld\" nkey=\"%lld\" >\n",t->hash_nlog, t->hash_nkey);
   for(i=0;i<=MAXSIZE_HASH;i++) { 
    hbufp = &(t->hash[i]);
    if(hbufp->key != 0) { 
      kreg = KEY_REGION(hbufp->key);
      icall = KEY_CALL(hbufp->key);
      ibytes = KEY_BYTE(hbufp->key);
      irank = KEY_RANK(hbufp->key);
      if(icall <= 0 || icall > MAXSIZE_CALLS || ibytes < 0 || (irank > -1 && irank >= t->mpi_size)) {
   if(task.flags & DEBUG) {
 printf("IPM: %5d HENT_ERR key_test \n",task.mpi_rank);
 printf("IPM: %5d HENT_ERR key_test ",task.mpi_rank); IPM_SHOWBITS(hbufp->key); printf(" k=%lld\n",hbufp->key);
 printf("IPM: %5d HENT_ERR key_test ",task.mpi_rank); IPM_SHOWBITS(hbufp->key&KEY_MASK_REGION); printf(" k&region=%d\n",kreg);
 printf("IPM: %5d HENT_ERR key_test ",task.mpi_rank); IPM_SHOWBITS(hbufp->key&KEY_MASK_CALL); printf(" k&call=%d,%s\n",icall,task.call_label[icall]);
 printf("IPM: %5d HENT_ERR key_test ",task.mpi_rank); IPM_SHOWBITS(hbufp->key&KEY_MASK_RANK); printf(" k&rank=%d\n",irank);
 printf("IPM: %5d HENT_ERR key_test ",task.mpi_rank); IPM_SHOWBITS(hbufp->key&KEY_MASK_BYTE); printf(" k&byte=%d\n",ibytes);
 printf("IPM: %5d HENT_ERR key_test \n",task.mpi_rank);
  fflush(stdout);
    }
   }
 
   if(t->call_mask[icall] & CALL_SEM_RANK_NONE) {
    irank = -1;
   }
  
   if(irank > t->mpi_size) {
    irank = -1;
   }

   if(kreg <0 || kreg >= MAXSIZE_REGION) {
      printf("IPM: %d Hash error key=%lld region=%d > MAXSIZE_REGION\n", 
       t->mpi_rank,
       hbufp->key,
       KEY_REGION(hbufp->key)
      );
   }

#ifdef IPM_ENABLE_PROFLOW
   rv = ipmprintf(fh,buff_in,pflag,
	"<hent key=\"%lld\" key_last=\"%lld\" call=\"%s\" bytes=\"%d\" orank=\"%d\" region=\"%d\" count=\"%lld\" >%.4e %.4e %.4e</hent>\n",
	 hbufp->key,
	 hbufp->key_last,
	 t->call_label[icall],
	 ibytes,
	 irank,
	 KEY_REGION(hbufp->key),
	 hbufp->count,
	 hbufp->t_tot,
         hbufp->t_min,
         hbufp->t_max);
#else
      rv = ipmprintf(fh,buff_in,pflag,
	"<hent key=\"%lld\" call=\"%s\" bytes=\"%d\" orank=\"%d\" region=\"%d\" count=\"%lld\" >%.4e %.4e %.4e</hent>\n",
	 hbufp->key,
	 t->call_label[icall],
	 ibytes,
	 irank,
	 KEY_REGION(hbufp->key),
	 hbufp->count,
	 hbufp->t_tot,
	 hbufp->t_min,
         hbufp->t_max);
#endif
     }
    }
    rv = ipmprintf(fh,buff_in,pflag,"</hash>\n");
   }
   IPM_TIME_GTOD(stamp_current);
   rv = ipmprintf(fh,buff_in,pflag, "<internal rank=\"%d\" log_i=\"%12.6f\" log_t=\"%.4e\" report_delta=\"%.4e\" fname=\"%s\" logrank=\"%d\" ></internal>\n",
	t->mpi_rank,
	stamp_current,
	stamp_current-t->stamp_final,
        report_delta,
	j->log_fname,
	log_rank);

#ifdef IPM_MONITOR_PTHREADS
   {
     int i, j;
     
     rv = ipmprintf(fh,buff_in,pflag, "<pthreads>\n");

     for( i=0; i<MAXSIZE_NTHREADS; i++ )
       {
         rv = ipmprintf(fh,buff_in,pflag, "  <thread tid=%d mpirank=%d>", i, t->mpi_rank);

	 if( pthreads[i].active )
	   {
	     for( j=0; j<MAXSIZE_HPMCOUNTERS; j++ )
	       {
		 rv = ipmprintf(fh,buff_in,pflag, "%lld ",	     
				pthreads[i].papi_ctrs[j]);
	       }
	   }
	 else
	   {
	     rv = ipmprintf(fh,buff_in,pflag, "not active");
	   }
	 rv = ipmprintf(fh,buff_in,pflag, "</thread>\n");
	 
       }
   }
   rv = ipmprintf(fh,buff_in,pflag, "</pthreads>\n");
#endif

   rv = ipmprintf(fh,buff_in,pflag,"</task>\n\n");
   
   /* last task writes end string*/
   if(t->mpi_rank==(t->mpi_size-1)) {
     rv = ipmprintf(fh,buff_in,pflag, "</ipm_job_profile>\n");
   }
   /* return aggregate number of bytes written */
   return bsize;
}


static int lock_test(void) { /* anonymnous collective */
 FILE *fh;
 int fd,rv,log_rank=-1;
 double t0,t1,t2;
 char fn[MAXSIZE_FILENAME];

 sprintf(fn,"lockfile.%s",job.cookie);

 if(!task.taskid) {
  unlink(fn);
 }

 PMPI_Barrier(MPI_COMM_WORLD);
 IPM_TIME_GTOD(t0);
 IPM_FILE_LOCK(fn,fd);
 fh = fdopen(fd,"w+");
 IPM_TIME_GTOD(t1);
 rv = fscanf(fh,"%d", &log_rank);
 if(rv == 1) {
  log_rank ++;
 } else {
  log_rank = 0;
 }
 fseek(fh,0,SEEK_SET);
 fprintf(fh, "%d                 ", log_rank);
 IPM_FILE_UNLOCK(fn,fd);
 IPM_TIME_GTOD(t2);
 printf("IPM: %d lock_test log_rank=%d file=%s wait=%.6f got=%.6f done=%.6f\n",        task.taskid,
        log_rank,
        fn,
        t0,
        t1,
        t2);
 return 0;
}
