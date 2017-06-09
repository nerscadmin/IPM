
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include "mpi.h"

#ifdef USE_CBLAS
#include "cblas.h"
#elif USE_NVBLAS
#include "nvblas.h"
#elif USE_MKL
#include "mkl.h"
#endif

#define DGEMM_RESTRICT __restrict__

// ------------------------------------------------------- //
// Function: get_seconds
//
// Vendor may modify this call to provide higher resolution
// timing if required
// ------------------------------------------------------- //
double get_seconds() {
	struct timeval now;
	gettimeofday(&now, NULL);

	const double seconds = (double) now.tv_sec;
	const double usec    = (double) now.tv_usec;

	return seconds + (usec * 1.0e-6);
}

int N = 1536;

double alpha = 1.0;
double beta  = 1.0;

double* DGEMM_RESTRICT matrixA;
double* DGEMM_RESTRICT matrixB;
double* DGEMM_RESTRICT matrixC;

void dgemm(int repeats)
{

	printf("Populating with values...\n");
	int i, j, k, r;

	#pragma omp parallel for
	for(i = 0; i < N; i++) {
		for(j = 0; j < N; j++) {
			matrixA[i*N + j] = 2.0;
			matrixB[i*N + j] = 0.5;
			matrixC[i*N + j] = 1.0;
		}
	}

	printf("Performing multiplication...\n");

	const double start = get_seconds();

	// ------------------------------------------------------- //
	// VENDOR NOTIFICATION: START MODIFIABLE REGION
	//
	// Vendor is able to change the lines below to call optimized
	// DGEMM or other matrix multiplication routines. Do *NOT*
	// change any lines above this statement.
	// ------------------------------------------------------- //

	double sum = 0;

	// Repeat multiple times
	for(r = 0; r < repeats; r++) {
		#pragma omp parallel for private(sum)
		for(i = 0; i < N; i++) {
			for(j = 0; j < N; j++) {
				sum = 0;

				for(k = 0; k < N; k++) {
					sum += matrixA[i*N + k] * matrixB[k*N + j];
				}

				matrixC[i*N + j] = (alpha * sum) + (beta * matrixC[i*N + j]);
			}
		}
	}

	// ------------------------------------------------------- //
	// VENDOR NOTIFICATION: END MODIFIABLE REGION
	// ------------------------------------------------------- //

	// ------------------------------------------------------- //
	// DO NOT CHANGE CODE BELOW
	// ------------------------------------------------------- //

	const double end = get_seconds();

	printf("Calculating matrix check...\n");

	double final_sum = 0;
	long long int count     = 0;

	#pragma omp parallel for reduction(+:final_sum, count)
	for(i = 0; i < N; i++) {
		for(j = 0; j < N; j++) {
			final_sum += matrixC[i*N + j];
			count++;
		}
	}

	double N_dbl = (double) N;
	double matrix_memory = (3 * N_dbl * N_dbl) * ((double) sizeof(double));

	printf("\n");
	printf("===============================================================\n");

	const double count_dbl = (double) count;
	const double scaled_result = (final_sum / (count_dbl * repeats));

	printf("Final Sum is:         %f\n", scaled_result);

	const double check_sum = N_dbl + (1.0 / (double) (repeats));
	const double allowed_margin = 1.0e-8;

	if( (check_sum >= (scaled_result - allowed_margin)) &&
		(check_sum <= (scaled_result + allowed_margin)) ) {
		printf(" -> Solution check PASSED successfully.\n");
	} else {
		printf(" -> Solution check FAILED.\n");
	}

	printf("Memory for Matrices:  %f MB\n",
		(matrix_memory / (1024 * 1024)));

	const double time_taken = (end - start);

	printf("Multiply time:        %f seconds\n", time_taken);

	const double flops_computed = (N_dbl * N_dbl * N_dbl * 2.0 * (double)(repeats)) +
        	(N_dbl * N_dbl * 2 * (double)(repeats));

	printf("FLOPs computed:       %f\n", flops_computed);
	printf("GFLOP/s rate:         %f GF/s\n", (flops_computed / time_taken) / 1000000000.0);

	printf("===============================================================\n");
	printf("\n");

}


// ------------------------------------------------------- //
// Function: main
//
// Modify only in permitted regions (see comments in the
// function)
// ------------------------------------------------------- //
int main(int argc, char* argv[]) {
    MPI_Init(&argc, &argv);

	// ------------------------------------------------------- //
	// DO NOT CHANGE CODE BELOW
	// ------------------------------------------------------- //

    printf("Alpha =    %f\n", alpha);
    printf("Beta  =    %f\n", beta);

	if(N < 128) {
		printf("Error: N (%d) is less than 128, the matrix is too small.\n", N);
		exit(-1);
	}

	printf("Allocating Matrices...\n");
	matrixA = (double*) malloc(sizeof(double) * N * N);
	matrixB = (double*) malloc(sizeof(double) * N * N);
	matrixC = (double*) malloc(sizeof(double) * N * N);

    
    MPI_Pcontrol(1, "all_dgemm_calls");
    MPI_Pcontrol(1, "first_30_dgemm_calls");
    MPI_Pcontrol(1, "10_dgemm_calls");
    dgemm(10);
    MPI_Pcontrol(-1, "10_dgemm_call");
    MPI_Pcontrol(1, "20_dgemm_call");
    dgemm(20);
    MPI_Pcontrol(-1, "20_dgemm_call");
    MPI_Pcontrol(-1, "first_30_dgemm_calls");
    MPI_Pcontrol(1, "40_dgemm_call");
    dgemm(40);
    MPI_Pcontrol(-1, "40_dgemm_call");
    MPI_Pcontrol(-1, "all_dgemm_calls");


	free(matrixA);
	free(matrixB);
	free(matrixC);

    MPI_Finalize();
	return 0;
}

