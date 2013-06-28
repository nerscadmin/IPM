/*

 from stat mech    var(x) = <x^2>-<x>^2   [single pass]
 http://en.wikipedia.org/wiki/Computational_formula_for_the_variance
 http://en.wikipedia.org/wiki/Algorithms_for_calculating_variance
*/

 MPI_Op ipm_SUM_OP;
 MPI_Op ipm_MIN_OP;
 MPI_Op ipm_MAX_OP;


/* globally synchronizing collective */
int ipm_synopsis(void *sendbuf, void *recvbuf, int dim, int *N, int *n, char *datatype, char *op, int root) {
 int rv;
 int size, rank; 
 int W[3];

 if(dim <= 0 || dim > 3)  {
  printf("ipm_synopsis failed dim="%d,dim); exit(1); 
 }

 for(i=0;i<3;i++) {
 if(dim >= 1) {
  if(n[i] <= i) { printf("ipm_synopsis failed n[%d]="%d,n[%d],i,i); exit(1); }
  W[i] = N[i]/n[i];
  if(W[i]*n[i] != N[i]) { 
	printf("ipm_synopsis failed N[%d]\%n[%d] != 0",i,i); exit(1);
  }
 }

#ifdef HAVE_MPI
  PMPI_Comm_size( MPI_COMM_WORLD, &size);
  PMPI_Comm_rank( MPI_COMM_WORLD, &rank);
  if(!rank) {
  } else {
   PMPI_Send(
  }
#endif

 return rv;
}


