#ifndef IPM_CORE_H_INCLUDED
#define IPM_CORE_H_INCLUDED

#include "ipm_types.h"
#include "ipm_time.h"
#include "mod_selfmonitor.h"

#define STATE_NOTINIT         0
#define STATE_IN_INIT         1
#define STATE_ACTIVE          2
#define STATE_NOTACTIVE       3
#define STATE_IN_FINALIZE     4
#define STATE_FINALIZED       5
#define STATE_ERROR          99


/* 
 * IPM flags
 */
#define FLAG_DEBUG                (0x0000000000000001ULL <<  0)
#define FLAG_REPORT_NONE          (0x0000000000000001ULL <<  1)
#define FLAG_REPORT_TERSE         (0x0000000000000001ULL <<  2)
#define FLAG_REPORT_FULL          (0x0000000000000001ULL <<  3)
#define FLAG_LOG_NONE             (0x0000000000000001ULL <<  4)
#define FLAG_LOG_TERSE            (0x0000000000000001ULL <<  5)
#define FLAG_LOG_FULL             (0x0000000000000001ULL <<  6)
#define FLAG_OUTFILE              (0x0000000000000001ULL <<  7)
#define FLAG_LOGWRITER_POSIXIO    (0x0000000000000001ULL <<  8)
#define FLAG_LOGWRITER_MPIIO      (0x0000000000000001ULL <<  9)

/* is atexit() handler installed ? */
#define FLAG_USING_ATEXIT         (0x0000000000000001ULL <<  10)
#define FLAG_HPCNAME              (0x0000000000000001ULL <<  11)

/* report nested regions? */
#define FLAG_NESTED_REGIONS       (0x0000000000000001ULL <<  12)


/* clear all REPORT bits */
#define FLAG_CLEAR_REPORT(flags_)	\
  flags_ &= ~FLAG_REPORT_NONE;		\
  flags_ &= ~FLAG_REPORT_TERSE;		\
  flags_ &= ~FLAG_REPORT_FULL;


/* clear all LOG bits */
#define FLAG_CLEAR_LOG(flags_)		\
  flags_ &= ~FLAG_LOG_NONE;		\
  flags_ &= ~FLAG_LOG_TERSE;		\
  flags_ &= ~FLAG_LOG_FULL;

/* clear all LOG bits */
#define FLAG_CLEAR_LOGWRITER(flags_)		\
  flags_ &= ~FLAG_LOGWRITER_MPIIO;		\
  flags_ &= ~FLAG_LOGWRITER_POSIXIO	      


extern int ipm_state;

int ipm_init(int flags);
int ipm_finalize(int flags);

#endif /* IPM_CORE_H_INCLUDED */

