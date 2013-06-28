
#ifndef UTEST_H_INCLUDED
#define UTEST_H_INCLUDED

#define VERIFY(myrank_, call_, bytes_, orank_, region_, count_)		\
  fprintf(stdout, "%03d.VERIFY: call=\"%s\" bytes=\"%d\" orank=\"%d\" region=\"%d\" count=\"%d\"\n", \
	  myrank_, call_, bytes_, orank_, region_, count_);


#endif /* UTEST_H_INCLUDED */
