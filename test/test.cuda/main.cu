
#include <stdio.h>
#include <cuda.h>
#include <unistd.h>

#define REPEAT 10000

extern "C" __global__ void foo(float *a, int N)
{
  int i;
  
  for( i=0; i<REPEAT; i++ ) {
    int idx = blockIdx.x * blockDim.x + threadIdx.x;
    if (idx<N) 
      a[idx] = a[idx] * a[idx];
  }
}


extern "C" __global__ void bar(float *a, int N)
{
  int i;
  
  for( i=0; i<REPEAT/100; i++ ) {
    int idx = blockIdx.x * blockDim.x + threadIdx.x;
    if (idx<N) 
      a[idx] = a[idx] * a[idx];
  }
}

int main(int argc, char* argv[])
{
  const int N = 100000;   // Number of elements in arrays  
  float *a_h, *a_d;       // Pointer to host & device arrays
  float *b_h, *b_d;       // Pointer to host & device arrays
  size_t size = N * sizeof(float);
  

  a_h = (float *)malloc(size);        // Allocate array on host
  cudaMalloc((void **) &a_d, size);   // Allocate array on device

  b_h = (float *)malloc(size);        // Allocate array on host
  cudaMalloc((void **) &b_d, size);   // Allocate array on device
  
  // Initialize host array and copy it to CUDA device
  for (int i=0; i<N; i++) a_h[i] = 1+(float)1/i;
  

  // Do calculation on device:
  int block_size = 4;
  int n_blocks = N/block_size + (N%block_size == 0 ? 0:1);

  cudaMemcpy(a_d, a_h, size, cudaMemcpyHostToDevice);
  cudaMemcpy(b_d, b_h, size, cudaMemcpyHostToDevice);
  foo <<< n_blocks, block_size >>> (a_d, N);


  sleep(10);

  foo <<< n_blocks, block_size >>> (b_d, N);
  cudaMemcpy(a_h, a_d, sizeof(float)*N, cudaMemcpyDeviceToHost);
  cudaMemcpy(b_h, b_d, sizeof(float)*N, cudaMemcpyDeviceToHost);

  // Print results
  //  for (int i=0; i<N; i++) printf("%d %f\n", i, a_h[i]);
  // Cleanup
  free(a_h); cudaFree(a_d);
  free(b_h); cudaFree(b_d);
}
