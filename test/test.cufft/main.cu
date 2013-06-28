
#include <stdio.h>
#include <math.h>
#include <cuda.h>
#include <cuda_runtime.h>
#include <cufft.h>

#define NX      256
#define BATCH   10

int main(int argc, char *argv[])
{
  cufftHandle plan;
  cufftComplex *devPtr;
  cufftComplex data[NX*BATCH];
  int i;
  
  
  /* source data creation */
  for(i=  0 ; i < NX*BATCH ; i++){
    data[i].x = 1.0f;
    data[i].y = 1.0f;
  }

  /* GPU memory allocation */
  cudaMalloc((void**)&devPtr, sizeof(cufftComplex)*NX*BATCH);

  /* transfer to GPU memory */
  cudaMemcpy(devPtr, data, sizeof(cufftComplex)*NX*BATCH, cudaMemcpyHostToDevice);

  /* creates 1D FFT plan */
  cufftPlan1d(&plan, NX, CUFFT_C2C, BATCH);

  /* executes FFT processes */
  cufftExecC2C(plan, devPtr, devPtr, CUFFT_FORWARD);

  /* executes FFT processes (inverse transformation) */
  cufftExecC2C(plan, devPtr, devPtr, CUFFT_INVERSE);

  /* transfer results from GPU memory */
  cudaMemcpy(data, devPtr, sizeof(cufftComplex)*NX*BATCH, cudaMemcpyDeviceToHost);

  /* deletes CUFFT plan */
  cufftDestroy(plan);

  /* frees GPU memory */
  cudaFree(devPtr);

  for(i = 0 ; i < NX*BATCH ; i++){
    printf("data[%d] %f %f\n", i, data[i].x, data[i].y);
  }

  return 0;
}





