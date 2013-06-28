
/* use cartesian communicators? */
//#define USE_CART_COMM

/* use sendrecv or isend+recv? */
#define USE_SENDRECV 


#include <stdlib.h>
#include <stdio.h>
#include <mpi.h>

int jacobi_relax( double **u, double **utmp,
		  int sizex, int sizey )
{
  int i, j;
  double *tmp;

#pragma omp parallel for private(j) 
  for( i=1; i<sizey-1; i++ )
    {
      for( j=1; j<sizex-1; j++ )
	{
	  (*utmp)[i*sizex+j]=
	    0.25 * ( (*u)[(i+1)*sizex+j] + (*u)[(i-1)*sizex+j] +
		     (*u)[i*sizex+j+1] + (*u)[i*sizex+j-1]);
	}
    }

  /* switch u and utmp */
  tmp = *utmp;
  *utmp = *u;
  *u = tmp;

  
  return 0;
}


double jacobi_residual( double *u, int sizex, int sizey )
{
  int i, j;

  double res=0.0;
  double diff;
  for( i=1; i<sizey-1; i++ )
    {
      for( j=1; j<sizex-1; j++ )
        { 
	  diff = u[i*sizex+j] - 0.25 *
            ( u[(i+1)*sizex+j] + u[(i-1)*sizex+j] +
	      u[i*sizex+j+1] + u[i*sizex+j-1] );
	  res += diff*diff;
        }
    }

  return res;
}

void print_matrix();


int main( int argc, char* argv[] )
{
  /* resolution (number of internal points) 
     and number of iterations */
  int res, iter;

  /* number of processors in x and y direciton
     x == horizontal 
     y == vertical */
  int procx, procy; 

  /* total number of processes, 
     and our own process' rank */
  int nprocs, myrank;

  /* the ranks of the four neighbors */
  int top, bottom, left, right;

  MPI_Comm comm;
  int coords[2];
  int dim[2];

#ifdef USE_CART_COMM
  /* used for the construction of the 
     cartesian communicator */
  int ndims;
  int period[2];

#endif 

  /* each process knows the offsets and sizes of each 
     other processes, offsets and sizes are relative to
     the size of the matrix **excluding** the additional border
     (only taking into account the internal points */
  int *offsx, *offsy;
  int *sizex, *sizey;

  /* shorthand notations for sizex[myrank], sizey[myrank] */
  int mysizex, mysizey;
  
  /* gives the indices into local_u at wich the ghost 
     rows/colums or the actual colums/rows  values start
     (top, bottom, left, right) */
  int ght, ghb, ghl, ghr;
  int ut, ub, ul, ur;
  
  /* timing */
  double start, finish;

  double residual;

  /* global u, only master allocates it */
  double *u;

  /* each worker process allocates its local u */
  double *local_u;
  
  /* 'swap' array used by the jacobi solver */
  double *local_u_tmp;

  /* the master process creates a derived datatype for
     each worker process */
  MPI_Datatype *aug_submat;
  MPI_Datatype *submat;
  MPI_Datatype mysubmat;

  /* datatype for sending and receiveing ghost cells 
     we only need one for horizontal and one for vertical
     lines */
  MPI_Datatype hline, vline;
  
  MPI_Status status;
  int i, j, ret;

  /* for isen/recv */
  MPI_Status stats[4];
  MPI_Request reqs[4];

  /* ******************************************************* */

  MPI_Init( &argc, &argv );
  MPI_Comm_size( MPI_COMM_WORLD, &nprocs );
  MPI_Comm_rank( MPI_COMM_WORLD, &myrank );

  start = MPI_Wtime();

  res   = (argc>1?atoi(argv[1]):0);
  iter  = (argc>2?atoi(argv[2]):0);
  procx = (argc>3?atoi(argv[3]):0);
  procy = (argc>4?atoi(argv[4]):0);

  if( res<=0 || iter <=0 || procx <= 0 || procy <= 0 )
    {
      if( myrank==0 )
	{
	  fprintf(stderr,
		  "Usage: %s <res> <iter> <procx> <procy>\n",
		  argv[0]);
	}

      MPI_Abort(MPI_COMM_WORLD, 1);
    }


  /*
   * cannot continue if we have not enough processors
   */
  if( procx*procy>nprocs )
    {
      if( myrank==0 )
	{
	  fprintf(stderr, 
		  "Not enough processors for that topology (%dx%d)\n",
		  procx, procy );

	}
      MPI_Abort(MPI_COMM_WORLD, 1);
      return 1;
    }

  dim[0]=procx;
  dim[1]=procy;

#ifdef USE_CART_COMM
  /*
   * create a cartesian communicator with the specified topology
   */
  ndims=2;
  period[0]=period[1]=0;
  
  MPI_Cart_create( MPI_COMM_WORLD, ndims, 
		   dim, period, 1, &comm );
#else 
    comm = MPI_COMM_WORLD;
#endif

  /*
   * unused MPI processes can finish
   */
  if( comm==MPI_COMM_NULL )
    {
      MPI_Finalize();
      return 0;
    }


  /* determine new rank and comm. size*/
  MPI_Comm_rank( comm, &myrank );
  MPI_Comm_size( comm, &nprocs );

  /*
   * master process allocates and initializes global u
   */
  if( myrank==0 )
    {
      u=(double*) malloc( sizeof(double)*
			  (res+2)*(res+2) );

      /* initialize internal values of u */
      for( i=1; i<=res; i++ )
	{
	  for( j=1; j<=res; j++ )
	    {
	      u[i*(res+2)+j]=(i-1)*res+(j-1);
	    }
	}

      /* initialize border of u */
      for( i=0; i<res+2; i++ )
	{
	  /*top*/
	  u[i]=-1;

	  /*bottom*/
	  u[(res+1)*(res+2)+i]=-1;

	  /*left*/
	  u[i*(res+2)]=-1;

	  /*right*/
	  u[i*(res+2)+(res+1)]=-1;
	}

      /* printf("Global u of master process:\n"); */
      /* print_matrix( u, res+2, res+2 ); */
    }

  /* tell all the others the size and number of iterations*/
  MPI_Bcast( &res, 1, MPI_INT, 0, comm );

#ifdef USE_CART_COMM
  MPI_Cart_coords( comm, myrank, ndims, coords );
#else
  coords[0]=myrank%procx;
  coords[1]=myrank/procx;
#endif


  /* allocate space to hold each processors 
     offsets and sizes */
  sizex = (int*) malloc( sizeof(int)* nprocs );
  sizey = (int*) malloc( sizeof(int)* nprocs );
  offsx = (int*) malloc( sizeof(int)* nprocs );
  offsy = (int*) malloc( sizeof(int)* nprocs );

  /* compute our own offset ...*/
  offsx[myrank]=coords[0]*res/dim[0];
  offsy[myrank]=coords[1]*res/dim[1];

  /* ...and size */
  sizex[myrank]=(coords[0]+1)*res/dim[0]-offsx[myrank];
  sizey[myrank]=(coords[1]+1)*res/dim[1]-offsy[myrank];

  mysizex = sizex[myrank];
  mysizey = sizey[myrank];

  ght = 1;
  ghl = mysizex+2;
  ghr = 2*mysizex+3;
  ghb = (mysizey+1)*(mysizex+2)+1;

  ut = mysizex+2+1;
  ul = ut;
  ur = 2*mysizex+2;
  ub = (mysizex+2)*mysizey+1;
    

  /*  
  printf("Process %d at (%d,%d) offset (%d,%d) size (%d,%d)\n",
	 myrank, coords[0], coords[1], 
	 offsx[myrank], offsy[myrank],
	 mysizex, mysizey );
  */

  /* make sure each process knows the size and
     offset of the others */
  MPI_Allgather( &offsx[myrank], 1, MPI_INT,
		 offsx, 1, MPI_INT, comm );

  MPI_Allgather( &offsy[myrank], 1, MPI_INT, 
		 offsy, 1, MPI_INT, comm );

  MPI_Allgather( &mysizex, 1, MPI_INT,
		 sizex, 1, MPI_INT, comm );
  
  MPI_Allgather( &mysizey, 1, MPI_INT,
		 sizey, 1, MPI_INT, comm );

  /* each process allocates space for local u */
  local_u = (double*) malloc( sizeof(double)*
			      (mysizex+2)*
			      (mysizey+2) );

  local_u_tmp = (double*) malloc( sizeof(double)*
				  (mysizex+2)*
				  (mysizey+2) );
			       
  

  if( myrank==0 )
    {
      /* master creates a datatype for each worker process */

      aug_submat = (MPI_Datatype*) malloc( sizeof(MPI_Datatype)*nprocs );
      submat = (MPI_Datatype*) malloc( sizeof(MPI_Datatype)*nprocs );
      
      for( i=0; i<nprocs; i++ )
	{
	  /* datatype to send out sub-matrices of the global u matrix */
	  MPI_Type_vector( sizey[i]+2, sizex[i]+2, (res+2),
			   MPI_DOUBLE, &aug_submat[i] );
	  
	  MPI_Type_commit( &aug_submat[i] );

	
	  MPI_Type_vector( sizey[i], sizex[i], (res+2),
			   MPI_DOUBLE, &submat[i] );
	  
	  MPI_Type_commit( &submat[i] );
	
	}
    }


  /* create the datatypes for the exchange of the ghost 
     cells with the neighboring processors:
     vline is a vertical line (exchange top and bottom processor)
     hline is a horizontal line (exchange with left and right proc.)
     the datatype's stride is defined with respect to the local u array */
  MPI_Type_vector( mysizey, 1, mysizex+2,
		   MPI_DOUBLE, &vline );
  MPI_Type_vector( 1, mysizex, 0,
		   MPI_DOUBLE, &hline );

  MPI_Type_commit( &vline );
  MPI_Type_commit( &hline );

  /* find out neighbor processes */
#ifdef USE_CART_COMM
  MPI_Cart_shift( comm, 0, 1, &left, &right );
  MPI_Cart_shift( comm, 1, 1, &top, &bottom );
#else
  left   = (coords[0]==0)?MPI_PROC_NULL:(myrank-1);
  right  = (coords[0]==procx-1)?MPI_PROC_NULL:(myrank+1);
  bottom = (coords[1]==0)?MPI_PROC_NULL:(myrank-procx);
  top    = (coords[1]==procy-1)?MPI_PROC_NULL:(myrank+procx);

  /*
  fprintf(stderr, " (%d,%d)=%d left=%d right=%d top=%d bottom=%d\n", 
	  coords[0], coords[1], myrank, left, right, top, bottom );
  */

#endif

  MPI_Type_vector( mysizey, mysizex, (mysizex+2),
		   MPI_DOUBLE, &mysubmat );
  
  MPI_Type_commit( &mysubmat );

  if( myrank==0 )
    {
      for( i=1; i<nprocs; i++ )
	{
	  /* send out *one* element of the appropriate data-type 
	     to the worker process */

	  MPI_Send( &u[offsy[i]*(res+2)+offsx[i]],
		    1, aug_submat[i], i, 11, comm );
	}
      
      for( i=0; i<mysizey+2; i++ )
	{
	  for( j=0; j<mysizex+2; j++ )
	    {
	      local_u[i*(mysizex+2)+j] =
		u[i*(res+2)+j];
	    }
	}
    }
  else
    {
      MPI_Recv( local_u,
		(mysizex+2)*(mysizey+2),
		MPI_DOUBLE, 0, 11, comm, &status );
    }

  /*
  for( i=0; i<mysizey+2; i++ )
  {
  for( j=0; j<mysizex+2; j++ )
  {
  local_u[i*(mysizex+2)+j] = -myrank ;
  }
  } 
  */



  if( myrank==nprocs-1 )
    {
      // printf("\nLocal u of process with rank %d:\n", nprocs-1);
      // print_matrix( local_u, mysizex+2, mysizey+2 );
    }

  MPI_Pcontrol(0, "enter", "iterloop");
  //  MPI_Pcontrol(0, "enter iterloop");

  for( i=0; i<iter; i++ )
    {
      jacobi_relax( &local_u, &local_u_tmp, 
		    mysizex+2, mysizey+2 );

#ifdef USE_SENDRECV

      // get new top ghost cells
      MPI_Sendrecv( &local_u[ub], 1, hline, bottom, 22,
		    &local_u[ght], 1, hline, top, 22,
		    comm, &status );
      
      // get new bottom ghost cells 
      MPI_Sendrecv( &local_u[ut], 1, hline, top, 33,
		    &local_u[ghb], 1, hline, bottom, 33, 
		    comm, &status );
      
      // get new left ghost cells
      MPI_Sendrecv( &local_u[ur], 1, vline, right, 44,
		    &local_u[ghl], 1, vline, left, 44,
		    comm, &status );
      
      // get new right ghost cells
      MPI_Sendrecv( &local_u[ul], 1, vline, left, 55,
		    &local_u[ghr], 1, vline, right, 55,
		    comm, &status );

#else 
      MPI_Isend( &local_u[ub], 1, hline, bottom, 22, comm, &reqs[0] );
      MPI_Isend( &local_u[ut], 1, hline, top,    33, comm, &reqs[1] );
      MPI_Isend( &local_u[ur], 1, vline, right,  44, comm, &reqs[2] );
      MPI_Isend( &local_u[ul], 1, vline, left,   55, comm, &reqs[3] );

      MPI_Recv( &local_u[ght], 1, hline, top,    22, comm, &stats[0] );
      MPI_Recv( &local_u[ghb], 1, hline, bottom, 33, comm, &stats[1] );      
      MPI_Recv( &local_u[ghl], 1, vline, left,   44, comm, &stats[2] );
      MPI_Recv( &local_u[ghr], 1, vline, right,  55, comm, &stats[3] );

      MPI_Waitall(4, reqs, stats);
#endif 

      MPI_Barrier( comm );
    }

  MPI_Pcontrol(0, "exit", "iterloop"); 
  //MPI_Pcontrol(0, "exit iterloop");


  if( myrank==nprocs-1 )
    {
      // printf("\nLocal u of process with rank %d:\n", nprocs-1);
      // print_matrix( local_u, mysizex+2, mysizey+2 );
    }
  

  if( myrank==0 )
    {
      for( i=1; i<nprocs; i++ )
	{
	  /* receive *one* element of the appropriate data-type 
	     from the worker process */

	  MPI_Recv( &u[(offsy[i]+1)*(res+2)+offsx[i]+1],
		    1, submat[i], i, 11, comm, &status );
	}

      for( i=1; i<mysizey+1; i++ )
	{
	  for( j=1; j<mysizex+1; j++ )
	    {
	      u[i*(res+2)+j] =
		local_u[i*(mysizex+2)+j];
	    }
	}

    }
  else
    {
      MPI_Send( &local_u[mysizex+3], 1, mysubmat, 0, 11, comm );
    }


  if( myrank==0 )
    {
      residual = jacobi_residual( u, res+2, res+2 ); 
      /* print_matrix( u, res+2, res+2 ); */
    }

  finish = MPI_Wtime();
  
  if( myrank==0 )
    {
      printf( "%d,%d,%d,%d,%d,%g,%g\n", 
	      res, iter, procx, procy, 
	      procx*procy, finish-start, residual  );
    }


  MPI_Finalize();
}



void print_matrix( double *u, 
		   int sizex, int sizey )
{ 
  int i, j;

  for( i=0; i<sizey; i++ )
    {
      for( j=0; j<sizex; j++ )
	{
	  printf("%.2g ", u[i*sizex+j]);
	}
      printf("\n");
    }
}
