/*
 * Copyright 1993-2010 NVIDIA Corporation.  All rights reserved.
 *
 * NOTICE TO USER:   
 *
 * This source code is subject to NVIDIA ownership rights under U.S. and 
 * international Copyright laws.  Users and possessors of this source code 
 * are hereby granted a nonexclusive, royalty-free license to use this code 
 * in individual and commercial software.
 *
 * NVIDIA MAKES NO REPRESENTATION ABOUT THE SUITABILITY OF THIS SOURCE 
 * CODE FOR ANY PURPOSE.  IT IS PROVIDED "AS IS" WITHOUT EXPRESS OR 
 * IMPLIED WARRANTY OF ANY KIND.  NVIDIA DISCLAIMS ALL WARRANTIES WITH 
 * REGARD TO THIS SOURCE CODE, INCLUDING ALL IMPLIED WARRANTIES OF 
 * MERCHANTABILITY, NONINFRINGEMENT, AND FITNESS FOR A PARTICULAR PURPOSE.
 * IN NO EVENT SHALL NVIDIA BE LIABLE FOR ANY SPECIAL, INDIRECT, INCIDENTAL, 
 * OR CONSEQUENTIAL DAMAGES, OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS 
 * OF USE, DATA OR PROFITS,  WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE 
 * OR OTHER TORTIOUS ACTION,  ARISING OUT OF OR IN CONNECTION WITH THE USE 
 * OR PERFORMANCE OF THIS SOURCE CODE.  
 *
 * U.S. Government End Users.   This source code is a "commercial item" as 
 * that term is defined at  48 C.F.R. 2.101 (OCT 1995), consisting  of 
 * "commercial computer  software"  and "commercial computer software 
 * documentation" as such terms are  used in 48 C.F.R. 12.212 (SEPT 1995) 
 * and is provided to the U.S. Government only as a commercial end item.  
 * Consistent with 48 C.F.R.12.212 and 48 C.F.R. 227.7202-1 through 
 * 227.7202-4 (JUNE 1995), all U.S. Government End Users acquire the 
 * source code with only those rights set forth herein. 
 *
 * Any use of this source code in individual and commercial software must 
 * include, in the user documentation and internal comments to the code,
 * the above Disclaimer and U.S. Government End Users Notice.
 */

/*
 * This file contains example Fortran bindings for the CUBLAS library, These
 * bindings have been tested with Intel Fortran 9.0 on 32-bit and 64-bit 
 * Windows, and with g77 3.4.5 on 32-bit and 64-bit Linux. They will likely
 * have to be adjusted for other Fortran compilers and platforms.
 */

#include <ctype.h>
#include <stdio.h>
#include <string.h>
#include <stddef.h>
#include <stdlib.h>
#if defined(__GNUC__)
#include <stdint.h>
#endif /* __GNUC__ */
#include "cublas.h"   /* CUBLAS public header file  */

#include "fortran_common.h"
#include "fortran.h"


int CUBLAS_INIT (void) 
{
    return (int)cublasInit ();
}

int CUBLAS_SHUTDOWN (void) 
{
    return (int)cublasShutdown ();
}

int CUBLAS_ALLOC (const int *n, const int *elemSize, devptr_t *devicePtr)
{    
    void *tPtr;
    int retVal;
    retVal = (int)cublasAlloc (*n, *elemSize, &tPtr);
    *devicePtr = (devptr_t)(uintptr_t)tPtr;
    return retVal;
}

int CUBLAS_FREE (const devptr_t *devicePtr)
{
    void *tPtr;
    tPtr = (void *)(uintptr_t)(*devicePtr);
    return (int)cublasFree (tPtr);
}

int CUBLAS_SET_VECTOR (const int *n, const int *elemSize, const void *x,
                       const int *incx, const devptr_t *y, const int *incy)
{
    void *tPtr = (void *)(uintptr_t)(*y);
    return (int)cublasSetVector (*n, *elemSize, x, *incx, tPtr, *incy);
}

int CUBLAS_GET_VECTOR (const int *n, const int *elemSize, const devptr_t *x,
                       const int *incx, void *y, const int *incy)
{
    const void *tPtr = (const void *)(uintptr_t)(*x);
    return (int)cublasGetVector (*n, *elemSize, tPtr, *incx, y, *incy);
}

int CUBLAS_SET_MATRIX (const int *rows, const int *cols, const int *elemSize,
                       const void *A, const int *lda, const devptr_t *B, 
                       const int *ldb)
{
    void *tPtr = (void *)(uintptr_t)(*B);
    return (int)cublasSetMatrix (*rows, *cols, *elemSize, A, *lda, tPtr,*ldb);
}

int CUBLAS_GET_MATRIX (const int *rows, const int *cols, const int *elemSize,
                       const devptr_t *A, const int *lda, void *B, 
                       const int *ldb)
{
    const void *tPtr = (const void *)(uintptr_t)(*A);
    return (int)cublasGetMatrix (*rows, *cols, *elemSize, tPtr, *lda, B, *ldb);
}

int CUBLAS_GET_ERROR (void)
{
    return (int)cublasGetError();
}

void CUBLAS_XERBLA (const char *srName, int *info)
{
    cublasXerbla (srName, *info);
}



/*---------------------------------------------------------------------------*/
/*---------------------------------- BLAS1 ----------------------------------*/
/*---------------------------------------------------------------------------*/

int CUBLAS_ISAMAX (const int *n, const devptr_t *devPtrx, const int *incx)
{
    float *x = (float *)(uintptr_t)(*devPtrx);
    int retVal;
    retVal = cublasIsamax (*n, x, *incx);
    return retVal;
}

int CUBLAS_ISAMIN (const int *n, const devptr_t *devPtrx, const int *incx)
{
    float *x = (float *)(uintptr_t)(*devPtrx);
    int retVal;
    retVal = cublasIsamin (*n, x, *incx);
    return retVal;
}

#if CUBLAS_FORTRAN_COMPILER==CUBLAS_G77
double CUBLAS_SASUM (const int *n, const devptr_t *devPtrx, const int *incx)
#else
float CUBLAS_SASUM (const int *n, const devptr_t *devPtrx, const int *incx)
#endif
{
    float *x = (float *)(uintptr_t)(*devPtrx);
    float retVal;
    retVal = cublasSasum (*n, x, *incx);
    return retVal;
}

void CUBLAS_SAXPY (const int *n, const float *alpha, const devptr_t *devPtrx, 
                   const int *incx, const devptr_t *devPtry, const int *incy)
{
    float *x = (float *)(uintptr_t)(*devPtrx);
    float *y = (float *)(uintptr_t)(*devPtry);
    cublasSaxpy (*n, *alpha, x, *incx, y, *incy);
}

void CUBLAS_SCOPY (const int *n, const devptr_t *devPtrx, const int *incx, 
                   const devptr_t *devPtry, const int *incy)
{
    float *x = (float *)(uintptr_t)(*devPtrx);
    float *y = (float *)(uintptr_t)(*devPtry);
    cublasScopy (*n, x, *incx, y, *incy);
}

#if CUBLAS_FORTRAN_COMPILER==CUBLAS_G77
double CUBLAS_SDOT (const int *n, const devptr_t *devPtrx, const int *incx, 
                    const devptr_t *devPtry, const int *incy)
#else
float CUBLAS_SDOT (const int *n, const devptr_t *devPtrx, const int *incx, 
                   const devptr_t *devPtry, const int *incy)
#endif
{
    float *x = (float *)(uintptr_t)(*devPtrx);
    float *y = (float *)(uintptr_t)(*devPtry);
    return cublasSdot (*n, x, *incx, y, *incy);
}

#if CUBLAS_FORTRAN_COMPILER==CUBLAS_G77
double CUBLAS_SNRM2 (const int *n, const devptr_t *devPtrx, const int *incx)
#else
float CUBLAS_SNRM2 (const int *n, const devptr_t *devPtrx, const int *incx)
#endif
{
    float *x = (float *)(uintptr_t)(*devPtrx);
    return cublasSnrm2 (*n, x, *incx);
}

void CUBLAS_SROT (const int *n, const devptr_t *devPtrx, const int *incx, 
                  const devptr_t *devPtry, const int *incy, const float *sc, 
                  const float *ss)
{
    float *x = (float *)(uintptr_t)(*devPtrx);
    float *y = (float *)(uintptr_t)(*devPtry);
    cublasSrot (*n, x, *incx, y, *incy, *sc, *ss);
}

void CUBLAS_SROTG (float *sa, float *sb, float *sc, float *ss)
{
    cublasSrotg (sa, sb, sc, ss);
}

void CUBLAS_SROTM (const int *n, const devptr_t *devPtrx, const int *incx, 
                   const devptr_t *devPtry, const int *incy, 
                   const float* sparam) 
{
    float *x = (float *)(uintptr_t)(*devPtrx);
    float *y = (float *)(uintptr_t)(*devPtry);
    cublasSrotm (*n, x, *incx, y, *incy, sparam);
}

void CUBLAS_SROTMG (float *sd1, float *sd2, float *sx1, const float *sy1,
                    float* sparam)
{
    cublasSrotmg (sd1, sd2, sx1, sy1, sparam);
}

void CUBLAS_SSCAL (const int *n, const float *alpha, const devptr_t *devPtrx,
                   const int *incx)
{
    float *x = (float *)(uintptr_t)(*devPtrx);
    cublasSscal (*n, *alpha, x, *incx);
}

void CUBLAS_SSWAP (const int *n, const devptr_t *devPtrx, const int *incx, 
                   const devptr_t *devPtry, const int *incy)
{
    float *x = (float *)(uintptr_t)(*devPtrx);
    float *y = (float *)(uintptr_t)(*devPtry);
    cublasSswap (*n, x, *incx, y, *incy);
}

void CUBLAS_CAXPY (const int *n, const cuComplex *alpha, 
                   const devptr_t *devPtrx, const int *incx, 
                   const devptr_t *devPtry, const int *incy)
{
    cuComplex *x = (cuComplex *)(uintptr_t)(*devPtrx);
    cuComplex *y = (cuComplex *)(uintptr_t)(*devPtry);
    cublasCaxpy (*n, *alpha, x, *incx, y, *incy);
}

void CUBLAS_ZAXPY (const int *n, const cuDoubleComplex *alpha, 
                   const devptr_t *devPtrx, const int *incx, 
                   const devptr_t *devPtry, const int *incy)
{
    cuDoubleComplex *x = (cuDoubleComplex *)(uintptr_t)(*devPtrx);
    cuDoubleComplex *y = (cuDoubleComplex *)(uintptr_t)(*devPtry);
    cublasZaxpy (*n, *alpha, x, *incx, y, *incy);
}

void CUBLAS_CCOPY (const int *n, const devptr_t *devPtrx, const int *incx,
                   const devptr_t *devPtry, const int *incy)
{
    cuComplex *x = (cuComplex *)(uintptr_t)(*devPtrx);
    cuComplex *y = (cuComplex *)(uintptr_t)(*devPtry);
    cublasCcopy (*n, x, *incx, y, *incy);
}
void CUBLAS_ZCOPY (const int *n, const devptr_t *devPtrx, const int *incx,
                   const devptr_t *devPtry, const int *incy)
{
    cuDoubleComplex *x = (cuDoubleComplex *)(uintptr_t)(*devPtrx);
    cuDoubleComplex *y = (cuDoubleComplex *)(uintptr_t)(*devPtry);
    cublasZcopy (*n, x, *incx, y, *incy);
}
void CUBLAS_CROT (const int *n, const devptr_t *devPtrx, const int *incx, 
                  const devptr_t *devPtry, const int *incy, const float *sc, 
                  const cuComplex *cs)
{
    cuComplex *x = (cuComplex *)(uintptr_t)(*devPtrx);
    cuComplex *y = (cuComplex *)(uintptr_t)(*devPtry);
    cublasCrot (*n, x, *incx, y, *incy, *sc, *cs);
}

void CUBLAS_ZROT (const int *n, const devptr_t *devPtrx, const int *incx, 
                  const devptr_t *devPtry, const int *incy, const double *sc, 
                  const cuDoubleComplex *cs)
{
    cuDoubleComplex *x = (cuDoubleComplex *)(uintptr_t)(*devPtrx);
    cuDoubleComplex *y = (cuDoubleComplex *)(uintptr_t)(*devPtry);
    cublasZrot (*n, x, *incx, y, *incy, *sc, *cs);
}

void CUBLAS_CROTG (cuComplex *ca, const cuComplex *cb, float *sc,
                   cuComplex *cs)
{
    cublasCrotg (ca, *cb, sc, cs);
}

void CUBLAS_ZROTG (cuDoubleComplex *ca, const cuDoubleComplex *cb, double *sc,
                   cuDoubleComplex *cs)
{
    cublasZrotg (ca, *cb, sc, cs);
}

void CUBLAS_CSCAL (const int *n, const cuComplex *alpha, 
                   const devptr_t *devPtrx, const int *incx)
{
    cuComplex *x = (cuComplex *)(uintptr_t)(*devPtrx);
    cublasCscal (*n, *alpha, x, *incx);
}

void CUBLAS_CSROT (const int *n, const devptr_t *devPtrx, const int *incx, 
                   const devptr_t *devPtry, const int *incy, const float *sc, 
                   const float *ss)
{
    cuComplex *x = (cuComplex *)(uintptr_t)(*devPtrx);
    cuComplex *y = (cuComplex *)(uintptr_t)(*devPtry);
    cublasCsrot (*n, x, *incx, y, *incy, *sc, *ss);
}

void CUBLAS_ZDROT (const int *n, const devptr_t *devPtrx, const int *incx, 
                   const devptr_t *devPtry, const int *incy, const double *sc, 
                   const double *ss)
{
    cuDoubleComplex *x = (cuDoubleComplex *)(uintptr_t)(*devPtrx);
    cuDoubleComplex *y = (cuDoubleComplex *)(uintptr_t)(*devPtry);
    cublasZdrot (*n, x, *incx, y, *incy, *sc, *ss);
}

void CUBLAS_CSSCAL (const int *n, const float *alpha, const devptr_t *devPtrx,
                    const int *incx)
{
    cuComplex *x = (cuComplex *)(uintptr_t)(*devPtrx);
    cublasCsscal (*n, *alpha, x, *incx);
}

void CUBLAS_CSWAP (const int *n, const devptr_t *devPtrx, const int *incx,
                   const devptr_t *devPtry, const int *incy)
{
    cuComplex *x = (cuComplex *)(uintptr_t)(*devPtrx);
    cuComplex *y = (cuComplex *)(uintptr_t)(*devPtry);
    cublasCswap (*n, x, *incx, y, *incy);
}

void CUBLAS_ZSWAP (const int *n, const devptr_t *devPtrx, const int *incx,
                   const devptr_t *devPtry, const int *incy)
{
    cuDoubleComplex *x = (cuDoubleComplex *)(uintptr_t)(*devPtrx);
    cuDoubleComplex *y = (cuDoubleComplex *)(uintptr_t)(*devPtry);
    cublasZswap (*n, x, *incx, y, *incy);
}

void CUBLAS_CTRMV (const char *uplo, const char *trans, const char *diag,
                   const int *n, const devptr_t *devPtrA, const int *lda,
                   const devptr_t *devPtrx, const int *incx)
{
    cuComplex *A = (cuComplex *)(uintptr_t)(*devPtrA);
    cuComplex *x = (cuComplex *)(uintptr_t)(*devPtrx);       
    cublasCtrmv (uplo[0], trans[0], diag[0], *n, A, *lda, x, *incx);
}

void CUBLAS_ZTRMV (const char *uplo, const char *trans, const char *diag,
                   const int *n, const devptr_t *devPtrA, const int *lda,
                   const devptr_t *devPtrx, const int *incx)
{
    cuDoubleComplex *A = (cuDoubleComplex *)(uintptr_t)(*devPtrA);
    cuDoubleComplex *x = (cuDoubleComplex *)(uintptr_t)(*devPtrx);       
    cublasZtrmv (uplo[0], trans[0], diag[0], *n, A, *lda, x, *incx);
}
#ifdef RETURN_COMPLEX
cuComplex CUBLAS_CDOTU ( const int *n, const devptr_t *devPtrx,
                   const int *incx, const devptr_t *devPtry,const int *incy)
{
    cuComplex *x = (cuComplex *)(uintptr_t)(*devPtrx);
    cuComplex *y = (cuComplex *)(uintptr_t)(*devPtry);
    cuComplex retVal = cublasCdotu (*n, x, *incx, y, *incy);
    return retVal;
}
#else
void CUBLAS_CDOTU (cuComplex *retVal, const int *n, const devptr_t *devPtrx,
                   const int *incx, const devptr_t *devPtry,const int *incy)
{
    cuComplex *x = (cuComplex *)(uintptr_t)(*devPtrx);
    cuComplex *y = (cuComplex *)(uintptr_t)(*devPtry);
    *retVal = cublasCdotu (*n, x, *incx, y, *incy);
}
#endif
#ifdef RETURN_COMPLEX
cuComplex CUBLAS_CDOTC ( const int *n, const devptr_t *devPtrx,
                   const int *incx, const devptr_t *devPtry, const int *incy)
{
    cuComplex *x = (cuComplex *)(uintptr_t)(*devPtrx);
    cuComplex *y = (cuComplex *)(uintptr_t)(*devPtry);
    cuComplex retVal = cublasCdotc (*n, x, *incx, y, *incy);
    return retVal;
}
#else
void CUBLAS_CDOTC (cuComplex *retVal, const int *n, const devptr_t *devPtrx,
                   const int *incx, const devptr_t *devPtry, const int *incy)
{
    cuComplex *x = (cuComplex *)(uintptr_t)(*devPtrx);
    cuComplex *y = (cuComplex *)(uintptr_t)(*devPtry);
    *retVal = cublasCdotc (*n, x, *incx, y, *incy);
}
#endif
int CUBLAS_ICAMAX (const int *n, const devptr_t *devPtrx, const int *incx)
{
    cuComplex *x = (cuComplex *)(uintptr_t)(*devPtrx);
    return cublasIcamax (*n, x, *incx);
}

int CUBLAS_ICAMIN (const int *n, const devptr_t *devPtrx, const int *incx)
{
    cuComplex *x = (cuComplex *)(uintptr_t)(*devPtrx);
    return cublasIcamin (*n, x, *incx);
}

int CUBLAS_IZAMAX (const int *n, const devptr_t *devPtrx, const int *incx)
{
    cuDoubleComplex *x = (cuDoubleComplex *)(uintptr_t)(*devPtrx);
    return cublasIzamax (*n, x, *incx);
}

int CUBLAS_IZAMIN (const int *n, const devptr_t *devPtrx, const int *incx)
{
    cuDoubleComplex *x = (cuDoubleComplex *)(uintptr_t)(*devPtrx);
    return cublasIzamin (*n, x, *incx);
}

#if CUBLAS_FORTRAN_COMPILER==CUBLAS_G77
double CUBLAS_SCASUM (const int *n, const devptr_t *devPtrx, const int *incx)
#else
float CUBLAS_SCASUM (const int *n, const devptr_t *devPtrx, const int *incx)
#endif
{
    cuComplex *x = (cuComplex *)(uintptr_t)(*devPtrx);
    return cublasScasum (*n, x, *incx);
}

double CUBLAS_DZASUM (const int *n, const devptr_t *devPtrx, const int *incx)
{
    cuDoubleComplex *x = (cuDoubleComplex *)(uintptr_t)(*devPtrx);
    return cublasDzasum (*n, x, *incx);
}

#if CUBLAS_FORTRAN_COMPILER==CUBLAS_G77
double CUBLAS_SCNRM2 (const int *n, const devptr_t *devPtrx, const int *incx)
#else
float CUBLAS_SCNRM2 (const int *n, const devptr_t *devPtrx, const int *incx)
#endif
{
    cuComplex *x = (cuComplex *)(uintptr_t)(*devPtrx);
    return cublasScnrm2 (*n, x, *incx);
}

double CUBLAS_DZNRM2 (const int *n, const devptr_t *devPtrx, const int *incx)
{
    cuDoubleComplex *x = (cuDoubleComplex *)(uintptr_t)(*devPtrx);
    return cublasDznrm2 (*n, x, *incx);
}

int CUBLAS_IDAMAX (const int *n, const devptr_t *devPtrx, const int *incx)
{
    double *x = (double *)(uintptr_t)(*devPtrx);
    int retVal;
    retVal = cublasIdamax (*n, x, *incx);
    return retVal;
}

int CUBLAS_IDAMIN (const int *n, const devptr_t *devPtrx, const int *incx)
{
    double *x = (double *)(uintptr_t)(*devPtrx);
    int retVal;
    retVal = cublasIdamin (*n, x, *incx);
    return retVal;
}

double CUBLAS_DASUM (const int *n, const devptr_t *devPtrx, const int *incx)
{
    double *x = (double *)(uintptr_t)(*devPtrx);
    double retVal;
    retVal = cublasDasum (*n, x, *incx);
    return retVal;
}

void CUBLAS_DAXPY (const int *n, const double *alpha, const devptr_t *devPtrx, 
                   const int *incx, const devptr_t *devPtry, const int *incy)
{
    double *x = (double *)(uintptr_t)(*devPtrx);
    double *y = (double *)(uintptr_t)(*devPtry);
    cublasDaxpy (*n, *alpha, x, *incx, y, *incy);
}

void CUBLAS_DCOPY (const int *n, const devptr_t *devPtrx, const int *incx, 
                   const devptr_t *devPtry, const int *incy)
{
    double *x = (double *)(uintptr_t)(*devPtrx);
    double *y = (double *)(uintptr_t)(*devPtry);
    cublasDcopy (*n, x, *incx, y, *incy);
}

double CUBLAS_DDOT (const int *n, const devptr_t *devPtrx, const int *incx, 
                    const devptr_t *devPtry, const int *incy)
{
    double *x = (double *)(uintptr_t)(*devPtrx);
    double *y = (double *)(uintptr_t)(*devPtry);
    return cublasDdot (*n, x, *incx, y, *incy);
}

double CUBLAS_DNRM2 (const int *n, const devptr_t *devPtrx, const int *incx)
{
    double *x = (double *)(uintptr_t)(*devPtrx);
    return cublasDnrm2 (*n, x, *incx);
}

void CUBLAS_DROT (const int *n, const devptr_t *devPtrx, const int *incx, 
                  const devptr_t *devPtry, const int *incy, const double *sc, 
                  const double *ss)
{
    double *x = (double *)(uintptr_t)(*devPtrx);
    double *y = (double *)(uintptr_t)(*devPtry);
    cublasDrot (*n, x, *incx, y, *incy, *sc, *ss);
}

void CUBLAS_DROTG (double *sa, double *sb, double *sc, double *ss)
{
    cublasDrotg (sa, sb, sc, ss);
}

void CUBLAS_DROTM (const int *n, const devptr_t *devPtrx, const int *incx, 
                   const devptr_t *devPtry, const int *incy, 
                   const double* sparam) 
{
    double *x = (double *)(uintptr_t)(*devPtrx);
    double *y = (double *)(uintptr_t)(*devPtry);
    cublasDrotm (*n, x, *incx, y, *incy, sparam);
}

void CUBLAS_DROTMG (double *sd1, double *sd2, double *sx1, const double *sy1,
                    double* sparam)
{
    cublasDrotmg (sd1, sd2, sx1, sy1, sparam);
}

void CUBLAS_DSCAL (const int *n, const double *alpha, const devptr_t *devPtrx,
                   const int *incx)
{
    double *x = (double *)(uintptr_t)(*devPtrx);
    cublasDscal (*n, *alpha, x, *incx);
}

void CUBLAS_DSWAP (const int *n, const devptr_t *devPtrx, const int *incx, 
                   const devptr_t *devPtry, const int *incy)
{
    double *x = (double *)(uintptr_t)(*devPtrx);
    double *y = (double *)(uintptr_t)(*devPtry);
    cublasDswap (*n, x, *incx, y, *incy);
}
#ifdef RETURN_COMPLEX
cuDoubleComplex CUBLAS_ZDOTU ( const int *n, const devptr_t *devPtrx,
                   const int *incx, const devptr_t *devPtry,const int *incy)
{
    cuDoubleComplex *x = (cuDoubleComplex *)(uintptr_t)(*devPtrx);
    cuDoubleComplex *y = (cuDoubleComplex *)(uintptr_t)(*devPtry);
    return (cublasZdotu (*n, x, *incx, y, *incy));
}
#else
void CUBLAS_ZDOTU (cuDoubleComplex *retVal, const int *n, const devptr_t *devPtrx,
                   const int *incx, const devptr_t *devPtry,const int *incy)
{
    cuDoubleComplex *x = (cuDoubleComplex *)(uintptr_t)(*devPtrx);
    cuDoubleComplex *y = (cuDoubleComplex *)(uintptr_t)(*devPtry);
    *retVal = cublasZdotu (*n, x, *incx, y, *incy);
}
#endif
#ifdef RETURN_COMPLEX
cuDoubleComplex CUBLAS_ZDOTC ( const int *n, const devptr_t *devPtrx,
                   const int *incx, const devptr_t *devPtry,const int *incy)
{
    cuDoubleComplex *x = (cuDoubleComplex *)(uintptr_t)(*devPtrx);
    cuDoubleComplex *y = (cuDoubleComplex *)(uintptr_t)(*devPtry);
    return (cublasZdotc (*n, x, *incx, y, *incy));
}
#else
void CUBLAS_ZDOTC (cuDoubleComplex *retVal, const int *n, const devptr_t *devPtrx,
                   const int *incx, const devptr_t *devPtry,const int *incy)
{
    cuDoubleComplex *x = (cuDoubleComplex *)(uintptr_t)(*devPtrx);
    cuDoubleComplex *y = (cuDoubleComplex *)(uintptr_t)(*devPtry);
    *retVal = cublasZdotc (*n, x, *incx, y, *incy);
}
#endif
void CUBLAS_ZSCAL (const int *n, const cuDoubleComplex *alpha, 
                   const devptr_t *devPtrx, const int *incx)
{
    cuDoubleComplex *x = (cuDoubleComplex *)(uintptr_t)(*devPtrx);
    cublasZscal (*n, *alpha, x, *incx);
}

void CUBLAS_ZDSCAL (const int *n, const double *alpha, const devptr_t *devPtrx,
                    const int *incx)
{
    cuDoubleComplex *x = (cuDoubleComplex *)(uintptr_t)(*devPtrx);
    cublasZdscal (*n, *alpha, x, *incx);
}

/*---------------------------------------------------------------------------*/
/*---------------------------------- BLAS2 ----------------------------------*/
/*---------------------------------------------------------------------------*/

void CUBLAS_SGBMV (const char *trans, const int *m, const int *n, 
                   const int *kl, const int *ku, const float *alpha,
                   const devptr_t *devPtrA, const int *lda, 
                   const devptr_t *devPtrx, const int *incx, const float *beta,
                   const devptr_t *devPtry, const int *incy)
{
    float *A = (float *)(uintptr_t)(*devPtrA);
    float *x = (float *)(uintptr_t)(*devPtrx);
    float *y = (float *)(uintptr_t)(*devPtry);
    cublasSgbmv (trans[0], *m, *n, *kl, *ku, *alpha, A, *lda, x, *incx, *beta,
                 y, *incy);
}

void CUBLAS_DGBMV (const char *trans, const int *m, const int *n,
                   const int *kl, const int *ku, const double *alpha, 
                   const devptr_t *devPtrA, const int *lda, 
                   const devptr_t *devPtrx, const int *incx, const double *beta,
                   const devptr_t *devPtry, const int *incy)
{
    double *A = (double *)(uintptr_t)(*devPtrA);
    double *x = (double *)(uintptr_t)(*devPtrx);
    double *y = (double *)(uintptr_t)(*devPtry);
    cublasDgbmv (trans[0], *m, *n, *kl, *ku, *alpha, A, *lda, x, *incx, *beta,
                 y, *incy);
}                   
void CUBLAS_CGBMV (const char *trans, const int *m, const int *n,
                   const int *kl, const int *ku, const cuComplex *alpha, 
                   const devptr_t *devPtrA, const int *lda, 
                   const devptr_t *devPtrx, const int *incx, const cuComplex *beta,
                   const devptr_t *devPtry, const int *incy)
{
    cuComplex *A = (cuComplex *)(uintptr_t)(*devPtrA);
    cuComplex *x = (cuComplex *)(uintptr_t)(*devPtrx);
    cuComplex *y = (cuComplex *)(uintptr_t)(*devPtry);
    cublasCgbmv (trans[0], *m, *n, *kl, *ku, *alpha, A, *lda, x, *incx, *beta,
                 y, *incy);
}                   
void CUBLAS_ZGBMV (const char *trans, const int *m, const int *n,
                   const int *kl, const int *ku, const cuDoubleComplex *alpha, 
                   const devptr_t *devPtrA, const int *lda, 
                   const devptr_t *devPtrx, const int *incx, const cuDoubleComplex *beta,
                   const devptr_t *devPtry, const int *incy)
{
    cuDoubleComplex *A = (cuDoubleComplex *)(uintptr_t)(*devPtrA);
    cuDoubleComplex *x = (cuDoubleComplex *)(uintptr_t)(*devPtrx);
    cuDoubleComplex *y = (cuDoubleComplex *)(uintptr_t)(*devPtry);
    cublasZgbmv (trans[0], *m, *n, *kl, *ku, *alpha, A, *lda, x, *incx, *beta,
                 y, *incy);
}                   

void CUBLAS_SGEMV (const char *trans, const int *m, const int *n, 
                   const float *alpha, const devptr_t *devPtrA, const int *lda,
                   const devptr_t *devPtrx, const int *incx, const float *beta,
                   const devptr_t *devPtry, const int *incy)
{
    float *A = (float *)(uintptr_t)(*devPtrA);
    float *x = (float *)(uintptr_t)(*devPtrx);
    float *y = (float *)(uintptr_t)(*devPtry);
    cublasSgemv (trans[0], *m, *n, *alpha, A, *lda, x, *incx, *beta, y, *incy);
}

void CUBLAS_SGER (const int *m, const int *n, const float *alpha, 
                  const devptr_t *devPtrx, const int *incx,
                  const devptr_t *devPtry, const int *incy,
                  const devptr_t *devPtrA, const int *lda)
{
    float *A = (float *)(uintptr_t)(*devPtrA);
    float *x = (float *)(uintptr_t)(*devPtrx);
    float *y = (float *)(uintptr_t)(*devPtry);    
    cublasSger (*m, *n, *alpha, x, *incx, y, *incy, A, *lda);
}

void CUBLAS_SSBMV (const char *uplo, const int *n, const int *k,
                   const float *alpha, const devptr_t *devPtrA, const int *lda,
                   const devptr_t *devPtrx, const int *incx, const float *beta,
                   const devptr_t *devPtry, const int *incy)
{
    float *A = (float *)(uintptr_t)(*devPtrA);
    float *x = (float *)(uintptr_t)(*devPtrx);
    float *y = (float *)(uintptr_t)(*devPtry);    
    cublasSsbmv (uplo[0], *n, *k, *alpha, A, *lda, x, *incx, *beta, y, *incy);
}

void CUBLAS_DSBMV (const char *uplo, const int *n, const int *k,
                   const double *alpha, const devptr_t *devPtrA, const int *lda,
                   const devptr_t *devPtrx, const int *incx, const double *beta,
                   const devptr_t *devPtry, const int *incy)
{
    double *A = (double *)(uintptr_t)(*devPtrA);
    double *x = (double *)(uintptr_t)(*devPtrx);
    double *y = (double *)(uintptr_t)(*devPtry);    
    cublasDsbmv (uplo[0], *n, *k, *alpha, A, *lda, x, *incx, *beta, y, *incy);
}

void CUBLAS_CHBMV (const char *uplo, const int *n, const int *k,
                   const cuComplex *alpha, const devptr_t *devPtrA, const int *lda,
                   const devptr_t *devPtrx, const int *incx, const cuComplex *beta,
                   const devptr_t *devPtry, const int *incy)
{
    cuComplex *A = (cuComplex *)(uintptr_t)(*devPtrA);
    cuComplex *x = (cuComplex *)(uintptr_t)(*devPtrx);
    cuComplex *y = (cuComplex *)(uintptr_t)(*devPtry);    
    cublasChbmv (uplo[0], *n, *k, *alpha, A, *lda, x, *incx, *beta, y, *incy);
}

void CUBLAS_ZHBMV (const char *uplo, const int *n, const int *k,
                   const cuDoubleComplex *alpha, const devptr_t *devPtrA, const int *lda,
                   const devptr_t *devPtrx, const int *incx, const cuDoubleComplex *beta,
                   const devptr_t *devPtry, const int *incy)
{
    cuDoubleComplex *A = (cuDoubleComplex *)(uintptr_t)(*devPtrA);
    cuDoubleComplex *x = (cuDoubleComplex *)(uintptr_t)(*devPtrx);
    cuDoubleComplex *y = (cuDoubleComplex *)(uintptr_t)(*devPtry);    
    cublasZhbmv (uplo[0], *n, *k, *alpha, A, *lda, x, *incx, *beta, y, *incy);
}

void CUBLAS_SSPMV (const char *uplo, const int *n, const float *alpha,
                   const devptr_t *devPtrAP, const devptr_t *devPtrx,
                   const int *incx, const float *beta, const devptr_t *devPtry,
                   const int *incy)
{
    float *AP = (float *)(uintptr_t)(*devPtrAP);
    float *x = (float *)(uintptr_t)(*devPtrx);
    float *y = (float *)(uintptr_t)(*devPtry);    
    cublasSspmv (uplo[0], *n, *alpha, AP, x, *incx, *beta, y, *incy);
}
void CUBLAS_DSPMV (const char *uplo, const int *n, const double *alpha,
                   const devptr_t *devPtrAP, const devptr_t *devPtrx,
                   const int *incx, const double *beta, const devptr_t *devPtry,
                   const int *incy)
{
    double *AP = (double *)(uintptr_t)(*devPtrAP);
    double *x = (double *)(uintptr_t)(*devPtrx);
    double *y = (double *)(uintptr_t)(*devPtry);    
    cublasDspmv (uplo[0], *n, *alpha, AP, x, *incx, *beta, y, *incy);
}
void CUBLAS_CHPMV (const char *uplo, const int *n, const cuComplex *alpha,
                   const devptr_t *devPtrAP, const devptr_t *devPtrx,
                   const int *incx, const cuComplex *beta, const devptr_t *devPtry,
                   const int *incy)
{
    cuComplex *AP = (cuComplex *)(uintptr_t)(*devPtrAP);
    cuComplex *x = (cuComplex *)(uintptr_t)(*devPtrx);
    cuComplex *y = (cuComplex *)(uintptr_t)(*devPtry);    
    cublasChpmv (uplo[0], *n, *alpha, AP, x, *incx, *beta, y, *incy);
}
void CUBLAS_ZHPMV (const char *uplo, const int *n, const cuDoubleComplex *alpha,
                   const devptr_t *devPtrAP, const devptr_t *devPtrx,
                   const int *incx, const cuDoubleComplex *beta, const devptr_t *devPtry,
                   const int *incy)
{
    cuDoubleComplex *AP = (cuDoubleComplex *)(uintptr_t)(*devPtrAP);
    cuDoubleComplex *x = (cuDoubleComplex *)(uintptr_t)(*devPtrx);
    cuDoubleComplex *y = (cuDoubleComplex *)(uintptr_t)(*devPtry);    
    cublasZhpmv (uplo[0], *n, *alpha, AP, x, *incx, *beta, y, *incy);
}

void CUBLAS_SSPR (const char *uplo, const int *n, const float *alpha, 
                  const devptr_t *devPtrx, const int *incx,
                  const devptr_t *devPtrAP)
{
    float *AP = (float *)(uintptr_t)(*devPtrAP);
    float *x = (float *)(uintptr_t)(*devPtrx);
    cublasSspr (uplo[0], *n, *alpha, x, *incx, AP);
}

void CUBLAS_DSPR (const char *uplo, const int *n, const double *alpha, 
                  const devptr_t *devPtrx, const int *incx,
                  const devptr_t *devPtrAP)
{
    double *AP = (double *)(uintptr_t)(*devPtrAP);
    double *x = (double *)(uintptr_t)(*devPtrx);
    cublasDspr (uplo[0], *n, *alpha, x, *incx, AP);
}

void CUBLAS_CHPR (const char *uplo, const int *n, const float *alpha, 
                  const devptr_t *devPtrx, const int *incx,
                  const devptr_t *devPtrAP)
{
    cuComplex *AP = (cuComplex *)(uintptr_t)(*devPtrAP);
    cuComplex *x = (cuComplex *)(uintptr_t)(*devPtrx);
    cublasChpr (uplo[0], *n, *alpha, x, *incx, AP);
}

void CUBLAS_ZHPR (const char *uplo, const int *n, const double *alpha, 
                  const devptr_t *devPtrx, const int *incx,
                  const devptr_t *devPtrAP)
{
    cuDoubleComplex *AP = (cuDoubleComplex *)(uintptr_t)(*devPtrAP);
    cuDoubleComplex *x = (cuDoubleComplex *)(uintptr_t)(*devPtrx);
    cublasZhpr (uplo[0], *n, *alpha, x, *incx, AP);
}


void CUBLAS_SSPR2 (const char *uplo, const int *n, const float *alpha,
                   const devptr_t *devPtrx, const int *incx, 
                   const devptr_t *devPtry, const int *incy,
                   const devptr_t *devPtrAP)
{
    float *AP = (float *)(uintptr_t)(*devPtrAP);
    float *x = (float *)(uintptr_t)(*devPtrx);
    float *y = (float *)(uintptr_t)(*devPtry);    
    cublasSspr2 (uplo[0], *n, *alpha, x, *incx, y, *incy, AP);
}

void CUBLAS_DSPR2 (const char *uplo, const int *n, const double *alpha,
                   const devptr_t *devPtrx, const int *incx, 
                   const devptr_t *devPtry, const int *incy,
                   const devptr_t *devPtrAP)
{
    double *AP = (double *)(uintptr_t)(*devPtrAP);
    double *x  = (double *)(uintptr_t)(*devPtrx);
    double *y  = (double *)(uintptr_t)(*devPtry);    
    cublasDspr2 (uplo[0], *n, *alpha, x, *incx, y, *incy, AP);
}

void CUBLAS_CHPR2 (const char *uplo, const int *n, const cuComplex *alpha,
                   const devptr_t *devPtrx, const int *incx, 
                   const devptr_t *devPtry, const int *incy,
                   const devptr_t *devPtrAP)
{
    cuComplex *AP = (cuComplex *)(uintptr_t)(*devPtrAP);
    cuComplex *x  = (cuComplex *)(uintptr_t)(*devPtrx);
    cuComplex *y  = (cuComplex *)(uintptr_t)(*devPtry);    
    cublasChpr2 (uplo[0], *n, *alpha, x, *incx, y, *incy, AP);
}

void CUBLAS_ZHPR2 (const char *uplo, const int *n, const cuDoubleComplex *alpha,
                   const devptr_t *devPtrx, const int *incx, 
                   const devptr_t *devPtry, const int *incy,
                   const devptr_t *devPtrAP)
{
    cuDoubleComplex *AP = (cuDoubleComplex *)(uintptr_t)(*devPtrAP);
    cuDoubleComplex *x  = (cuDoubleComplex *)(uintptr_t)(*devPtrx);
    cuDoubleComplex *y  = (cuDoubleComplex *)(uintptr_t)(*devPtry);    
    cublasZhpr2 (uplo[0], *n, *alpha, x, *incx, y, *incy, AP);
}

void CUBLAS_SSYMV (const char *uplo, const int *n, const float *alpha,
                   const devptr_t *devPtrA, const int *lda, 
                   const devptr_t *devPtrx, const int *incx, const float *beta,
                   const devptr_t *devPtry,
                   const int *incy)
{
    float *A = (float *)(uintptr_t)(*devPtrA);
    float *x = (float *)(uintptr_t)(*devPtrx);
    float *y = (float *)(uintptr_t)(*devPtry);    
    cublasSsymv (uplo[0], *n, *alpha, A, *lda, x, *incx, *beta, y, *incy);
}

void CUBLAS_DSYMV (const char *uplo, const int *n, const double *alpha,
                   const devptr_t *devPtrA, const int *lda, 
                   const devptr_t *devPtrx, const int *incx, const double *beta,
                   const devptr_t *devPtry,
                   const int *incy)
{
    double *A = (double *)(uintptr_t)(*devPtrA);
    double *x = (double *)(uintptr_t)(*devPtrx);
    double *y = (double *)(uintptr_t)(*devPtry);    
    cublasDsymv (uplo[0], *n, *alpha, A, *lda, x, *incx, *beta, y, *incy);
}

void CUBLAS_CHEMV (const char *uplo, const int *n, const cuComplex *alpha,
                   const devptr_t *devPtrA, const int *lda, 
                   const devptr_t *devPtrx, const int *incx, const cuComplex *beta,
                   const devptr_t *devPtry,
                   const int *incy)
{
    cuComplex *A = (cuComplex *)(uintptr_t)(*devPtrA);
    cuComplex *x = (cuComplex *)(uintptr_t)(*devPtrx);
    cuComplex *y = (cuComplex *)(uintptr_t)(*devPtry);    
    cublasChemv (uplo[0], *n, *alpha, A, *lda, x, *incx, *beta, y, *incy);
}

void CUBLAS_ZHEMV (const char *uplo, const int *n, const cuDoubleComplex *alpha,
                   const devptr_t *devPtrA, const int *lda, 
                   const devptr_t *devPtrx, const int *incx, const cuDoubleComplex *beta,
                   const devptr_t *devPtry,
                   const int *incy)
{
    cuDoubleComplex *A = (cuDoubleComplex *)(uintptr_t)(*devPtrA);
    cuDoubleComplex *x = (cuDoubleComplex *)(uintptr_t)(*devPtrx);
    cuDoubleComplex *y = (cuDoubleComplex *)(uintptr_t)(*devPtry);    
    cublasZhemv (uplo[0], *n, *alpha, A, *lda, x, *incx, *beta, y, *incy);
}

void CUBLAS_SSYR (const char *uplo, const int *n, const float *alpha,
                  const devptr_t *devPtrx, const int *incx,
                  const devptr_t *devPtrA, const int *lda)
{
    float *A = (float *)(uintptr_t)(*devPtrA);
    float *x = (float *)(uintptr_t)(*devPtrx);    
    cublasSsyr (uplo[0], *n, *alpha, x, *incx, A, *lda);
}

void CUBLAS_SSYR2 (const char *uplo, const int *n, const float *alpha,
                   const devptr_t *devPtrx, const int *incx,
                   const devptr_t *devPtry, const int *incy, 
                   const devptr_t *devPtrA, const int *lda)
{
    float *A = (float *)(uintptr_t)(*devPtrA);
    float *x = (float *)(uintptr_t)(*devPtrx);
    float *y = (float *)(uintptr_t)(*devPtry);    
    cublasSsyr2 (uplo[0], *n, *alpha, x, *incx, y, *incy, A, *lda);
}

void CUBLAS_DSYR2 (const char *uplo, const int *n, const double *alpha,
                   const devptr_t *devPtrx, const int *incx,
                   const devptr_t *devPtry, const int *incy, 
                   const devptr_t *devPtrA, const int *lda)
{
    double *A = (double *)(uintptr_t)(*devPtrA);
    double *x = (double *)(uintptr_t)(*devPtrx);
    double *y = (double *)(uintptr_t)(*devPtry);    
    cublasDsyr2 (uplo[0], *n, *alpha, x, *incx, y, *incy, A, *lda);
}

void CUBLAS_CHER2 (const char *uplo, const int *n, const cuComplex *alpha,
                   const devptr_t *devPtrx, const int *incx,
                   const devptr_t *devPtry, const int *incy, 
                   const devptr_t *devPtrA, const int *lda)
{
    cuComplex *A = (cuComplex *)(uintptr_t)(*devPtrA);
    cuComplex *x = (cuComplex *)(uintptr_t)(*devPtrx);
    cuComplex *y = (cuComplex *)(uintptr_t)(*devPtry);    
    cublasCher2 (uplo[0], *n, *alpha, x, *incx, y, *incy, A, *lda);
}

void CUBLAS_ZHER2 (const char *uplo, const int *n, const cuDoubleComplex *alpha,
                   const devptr_t *devPtrx, const int *incx,
                   const devptr_t *devPtry, const int *incy, 
                   const devptr_t *devPtrA, const int *lda)
{
    cuDoubleComplex *A = (cuDoubleComplex *)(uintptr_t)(*devPtrA);
    cuDoubleComplex *x = (cuDoubleComplex *)(uintptr_t)(*devPtrx);
    cuDoubleComplex *y = (cuDoubleComplex *)(uintptr_t)(*devPtry);    
    cublasZher2 (uplo[0], *n, *alpha, x, *incx, y, *incy, A, *lda);
}


void CUBLAS_STBMV (const char *uplo, const char *trans, const char *diag,
                   const int *n, const int *k, const devptr_t *devPtrA, 
                   const int *lda, const devptr_t *devPtrx, const int *incx)
{
    float *A = (float *)(uintptr_t)(*devPtrA);
    float *x = (float *)(uintptr_t)(*devPtrx);    
    cublasStbmv (uplo[0], trans[0], diag[0], *n, *k, A, *lda, x, *incx);
}

void CUBLAS_DTBMV (const char *uplo, const char *trans, const char *diag,
                   const int *n, const int *k, const devptr_t *devPtrA, 
                   const int *lda, const devptr_t *devPtrx, const int *incx)
{
    double *A = (double *)(uintptr_t)(*devPtrA);
    double *x = (double *)(uintptr_t)(*devPtrx);    
    cublasDtbmv (uplo[0], trans[0], diag[0], *n, *k, A, *lda, x, *incx);
}

void CUBLAS_CTBMV (const char *uplo, const char *trans, const char *diag,
                   const int *n, const int *k, const devptr_t *devPtrA, 
                   const int *lda, const devptr_t *devPtrx, const int *incx)
{
    cuComplex *A = (cuComplex *)(uintptr_t)(*devPtrA);
    cuComplex *x = (cuComplex *)(uintptr_t)(*devPtrx);    
    cublasCtbmv (uplo[0], trans[0], diag[0], *n, *k, A, *lda, x, *incx);
}

void CUBLAS_ZTBMV (const char *uplo, const char *trans, const char *diag,
                   const int *n, const int *k, const devptr_t *devPtrA, 
                   const int *lda, const devptr_t *devPtrx, const int *incx)
{
    cuDoubleComplex *A = (cuDoubleComplex *)(uintptr_t)(*devPtrA);
    cuDoubleComplex *x = (cuDoubleComplex *)(uintptr_t)(*devPtrx);    
    cublasZtbmv (uplo[0], trans[0], diag[0], *n, *k, A, *lda, x, *incx);
}

void CUBLAS_STBSV (const char *uplo, const char *trans, const char *diag,
                   const int *n, const int *k, const devptr_t *devPtrA, 
                   const int *lda, const devptr_t *devPtrx, const int *incx)
{
    float *A = (float *)(uintptr_t)(*devPtrA);
    float *x = (float *)(uintptr_t)(*devPtrx);       
    cublasStbsv (uplo[0], trans[0], diag[0], *n, *k, A, *lda, x, *incx);
}

void CUBLAS_DTBSV (const char *uplo, const char *trans, const char *diag,
                   const int *n, const int *k, const devptr_t *devPtrA, 
                   const int *lda, const devptr_t *devPtrx, const int *incx)
{
    double *A = (double *)(uintptr_t)(*devPtrA);
    double *x = (double *)(uintptr_t)(*devPtrx);       
    cublasDtbsv (uplo[0], trans[0], diag[0], *n, *k, A, *lda, x, *incx);
}

void CUBLAS_CTBSV (const char *uplo, const char *trans, const char *diag,
                   const int *n, const int *k, const devptr_t *devPtrA, 
                   const int *lda, const devptr_t *devPtrx, const int *incx)
{
    cuComplex *A = (cuComplex *)(uintptr_t)(*devPtrA);
    cuComplex *x = (cuComplex *)(uintptr_t)(*devPtrx);       
    cublasCtbsv (uplo[0], trans[0], diag[0], *n, *k, A, *lda, x, *incx);
}

void CUBLAS_ZTBSV (const char *uplo, const char *trans, const char *diag,
                   const int *n, const int *k, const devptr_t *devPtrA, 
                   const int *lda, const devptr_t *devPtrx, const int *incx)
{
    cuDoubleComplex *A = (cuDoubleComplex *)(uintptr_t)(*devPtrA);
    cuDoubleComplex *x = (cuDoubleComplex *)(uintptr_t)(*devPtrx);       
    cublasZtbsv (uplo[0], trans[0], diag[0], *n, *k, A, *lda, x, *incx);
}

void CUBLAS_STPMV (const char *uplo, const char *trans, const char *diag,
                   const int *n,  const devptr_t *devPtrAP, 
                   const devptr_t *devPtrx, const int *incx)
{
    float *AP = (float *)(uintptr_t)(*devPtrAP);
    float *x = (float *)(uintptr_t)(*devPtrx);       
    cublasStpmv (uplo[0], trans[0], diag[0], *n, AP, x, *incx);
}

void CUBLAS_DTPMV (const char *uplo, const char *trans, const char *diag,
                   const int *n,  const devptr_t *devPtrAP, 
                   const devptr_t *devPtrx, const int *incx)
{
    double *AP = (double *)(uintptr_t)(*devPtrAP);
    double *x = (double *)(uintptr_t)(*devPtrx);       
    cublasDtpmv (uplo[0], trans[0], diag[0], *n, AP, x, *incx);
}

void CUBLAS_CTPMV (const char *uplo, const char *trans, const char *diag,
                   const int *n,  const devptr_t *devPtrAP, 
                   const devptr_t *devPtrx, const int *incx)
{
    cuComplex *AP = (cuComplex *)(uintptr_t)(*devPtrAP);
    cuComplex *x = (cuComplex *)(uintptr_t)(*devPtrx);       
    cublasCtpmv (uplo[0], trans[0], diag[0], *n, AP, x, *incx);
}

void CUBLAS_ZTPMV (const char *uplo, const char *trans, const char *diag,
                   const int *n,  const devptr_t *devPtrAP, 
                   const devptr_t *devPtrx, const int *incx)
{
    cuDoubleComplex *AP = (cuDoubleComplex *)(uintptr_t)(*devPtrAP);
    cuDoubleComplex *x = (cuDoubleComplex *)(uintptr_t)(*devPtrx);       
    cublasZtpmv (uplo[0], trans[0], diag[0], *n, AP, x, *incx);
}

void CUBLAS_STPSV (const char *uplo, const char *trans, const char *diag,
                   const int *n, const devptr_t *devPtrAP, 
                   const devptr_t *devPtrx, const int *incx)
{
    float *AP = (float *)(uintptr_t)(*devPtrAP);
    float *x = (float *)(uintptr_t)(*devPtrx);       
    cublasStpsv (uplo[0], trans[0], diag[0], *n, AP, x, *incx);
}

void CUBLAS_DTPSV (const char *uplo, const char *trans, const char *diag,
                   const int *n, const devptr_t *devPtrAP, 
                   const devptr_t *devPtrx, const int *incx)
{
    double *AP = (double *)(uintptr_t)(*devPtrAP);
    double *x = (double *)(uintptr_t)(*devPtrx);       
    cublasDtpsv (uplo[0], trans[0], diag[0], *n, AP, x, *incx);
}

void CUBLAS_CTPSV (const char *uplo, const char *trans, const char *diag,
                   const int *n, const devptr_t *devPtrAP, 
                   const devptr_t *devPtrx, const int *incx)
{
    cuComplex *AP = (cuComplex *)(uintptr_t)(*devPtrAP);
    cuComplex *x = (cuComplex *)(uintptr_t)(*devPtrx);       
    cublasCtpsv (uplo[0], trans[0], diag[0], *n, AP, x, *incx);
}

void CUBLAS_ZTPSV (const char *uplo, const char *trans, const char *diag,
                   const int *n, const devptr_t *devPtrAP, 
                   const devptr_t *devPtrx, const int *incx)
{
    cuDoubleComplex *AP = (cuDoubleComplex *)(uintptr_t)(*devPtrAP);
    cuDoubleComplex *x = (cuDoubleComplex *)(uintptr_t)(*devPtrx);       
    cublasZtpsv (uplo[0], trans[0], diag[0], *n, AP, x, *incx);
}

void CUBLAS_STRMV (const char *uplo, const char *trans, const char *diag,
                   const int *n, const devptr_t *devPtrA, const int *lda,
                   const devptr_t *devPtrx, const int *incx)
{
    float *A = (float *)(uintptr_t)(*devPtrA);
    float *x = (float *)(uintptr_t)(*devPtrx);       
    cublasStrmv (uplo[0], trans[0], diag[0], *n, A, *lda, x, *incx);
}

void CUBLAS_DTRMV (const char *uplo, const char *trans, const char *diag,
                   const int *n, const devptr_t *devPtrA, const int *lda,
                   const devptr_t *devPtrx, const int *incx)
{
    double *A = (double *)(uintptr_t)(*devPtrA);
    double *x = (double *)(uintptr_t)(*devPtrx);       
    cublasDtrmv (uplo[0], trans[0], diag[0], *n, A, *lda, x, *incx);
}

void CUBLAS_STRSV (const char *uplo, const char *trans, const char *diag,
                   const int *n, const devptr_t *devPtrA, const int *lda,
                   const devptr_t *devPtrx, const int *incx)
{
    float *A = (float *)(uintptr_t)(*devPtrA);
    float *x = (float *)(uintptr_t)(*devPtrx);       
    cublasStrsv (uplo[0], trans[0], diag[0], *n, A, *lda, x, *incx);
}

void CUBLAS_DGEMV (const char *trans, const int *m, const int *n, 
                   const double *alpha, const devptr_t *devPtrA,
                   const int *lda, const devptr_t *devPtrx, const int *incx,
                   const double *beta, const devptr_t *devPtry,
                   const int *incy)
{
    double *A = (double *)(uintptr_t)(*devPtrA);
    double *x = (double *)(uintptr_t)(*devPtrx);
    double *y = (double *)(uintptr_t)(*devPtry);
    cublasDgemv (trans[0], *m, *n, *alpha, A, *lda, x, *incx, *beta, y, *incy);
}
void CUBLAS_CGEMV (const char *trans, const int *m, const int *n,
                   const cuComplex *alpha, const devptr_t *devPtrA,
                   const int *lda, const devptr_t *devPtrx, const int *incx,
                   const cuComplex *beta, devptr_t *devPtry,
                   const int *incy)
{
    cuComplex *A = (cuComplex *)(uintptr_t)(*devPtrA);
    cuComplex *x = (cuComplex *)(uintptr_t)(*devPtrx);
    cuComplex *y = (cuComplex *)(uintptr_t)(*devPtry);
    cublasCgemv (trans[0], *m, *n, *alpha, A, *lda, x, *incx, *beta, y, *incy);
}

void CUBLAS_ZGEMV (const char *trans, const int *m, const int *n,
                   const cuDoubleComplex *alpha, const devptr_t *devPtrA,
                   const int *lda, const devptr_t *devPtrx, const int *incx,
                   const cuDoubleComplex *beta, devptr_t *devPtry,
                   const int *incy)
{
    cuDoubleComplex *A = (cuDoubleComplex *)(uintptr_t)(*devPtrA);
    cuDoubleComplex *x = (cuDoubleComplex *)(uintptr_t)(*devPtrx);
    cuDoubleComplex *y = (cuDoubleComplex *)(uintptr_t)(*devPtry);
    cublasZgemv (trans[0], *m, *n, *alpha, A, *lda, x, *incx, *beta, y, *incy);
}
void CUBLAS_DGER (const int *m, const int *n, const double *alpha, 
                  const devptr_t *devPtrx, const int *incx,
                  const devptr_t *devPtry, const int *incy,
                  const devptr_t *devPtrA, const int *lda)
{
    double *A = (double *)(uintptr_t)(*devPtrA);
    double *x = (double *)(uintptr_t)(*devPtrx);
    double *y = (double *)(uintptr_t)(*devPtry);    
    cublasDger (*m, *n, *alpha, x, *incx, y, *incy, A, *lda);
}

void CUBLAS_DSYR (const char *uplo, const int *n, const double *alpha,
                  const devptr_t *devPtrx, const int *incx,
                  const devptr_t *devPtrA, const int *lda)
{
    double *A = (double *)(uintptr_t)(*devPtrA);
    double *x = (double *)(uintptr_t)(*devPtrx);    
    cublasDsyr (uplo[0], *n, *alpha, x, *incx, A, *lda);
}

void CUBLAS_CHER (const char *uplo, const int *n, const float *alpha,
                  const devptr_t *devPtrx, const int *incx,
                  const devptr_t *devPtrA, const int *lda)
{
    cuComplex *A = (cuComplex *)(uintptr_t)(*devPtrA);
    cuComplex *x = (cuComplex *)(uintptr_t)(*devPtrx);    
    cublasCher (uplo[0], *n, *alpha, x, *incx, A, *lda);
}

void CUBLAS_ZHER (const char *uplo, const int *n, const double *alpha,
                  const devptr_t *devPtrx, const int *incx,
                  const devptr_t *devPtrA, const int *lda)
{
    cuDoubleComplex *A = (cuDoubleComplex *)(uintptr_t)(*devPtrA);
    cuDoubleComplex *x = (cuDoubleComplex *)(uintptr_t)(*devPtrx);    
    cublasZher (uplo[0], *n, *alpha, x, *incx, A, *lda);
}

void CUBLAS_DTRSV (const char *uplo, const char *trans, const char *diag,
                   const int *n, const devptr_t *devPtrA, const int *lda,
                   const devptr_t *devPtrx, const int *incx)
{
    double *A = (double *)(uintptr_t)(*devPtrA);
    double *x = (double *)(uintptr_t)(*devPtrx);       
    cublasDtrsv (uplo[0], trans[0], diag[0], *n, A, *lda, x, *incx);
}

void CUBLAS_CTRSV (const char *uplo, const char *trans, const char *diag,
                   const int *n, const devptr_t *devPtrA, const int *lda,
                   const devptr_t *devPtrx, const int *incx)
{
    cuComplex *A = (cuComplex *)(uintptr_t)(*devPtrA);
    cuComplex *x = (cuComplex *)(uintptr_t)(*devPtrx);       
    cublasCtrsv (uplo[0], trans[0], diag[0], *n, A, *lda, x, *incx);
}

void CUBLAS_ZTRSV (const char *uplo, const char *trans, const char *diag,
                   const int *n, const devptr_t *devPtrA, const int *lda,
                   const devptr_t *devPtrx, const int *incx)
{
    cuDoubleComplex *A = (cuDoubleComplex *)(uintptr_t)(*devPtrA);
    cuDoubleComplex *x = (cuDoubleComplex *)(uintptr_t)(*devPtrx);       
    cublasZtrsv (uplo[0], trans[0], diag[0], *n, A, *lda, x, *incx);
}

void CUBLAS_CGERU (const int *m, const int *n, const cuComplex *alpha, 
                  const devptr_t *devPtrx, const int *incx,
                  const devptr_t *devPtry, const int *incy,
                  const devptr_t *devPtrA, const int *lda)
{
    cuComplex *A = (cuComplex *)(uintptr_t)(*devPtrA);
    cuComplex *x = (cuComplex *)(uintptr_t)(*devPtrx);
    cuComplex *y = (cuComplex *)(uintptr_t)(*devPtry);    
    cublasCgeru (*m, *n, *alpha, x, *incx, y, *incy, A, *lda);
}

void CUBLAS_CGERC (const int *m, const int *n, const cuComplex *alpha, 
                  const devptr_t *devPtrx, const int *incx,
                  const devptr_t *devPtry, const int *incy,
                  const devptr_t *devPtrA, const int *lda)
{
    cuComplex *A = (cuComplex *)(uintptr_t)(*devPtrA);
    cuComplex *x = (cuComplex *)(uintptr_t)(*devPtrx);
    cuComplex *y = (cuComplex *)(uintptr_t)(*devPtry);    
    cublasCgerc (*m, *n, *alpha, x, *incx, y, *incy, A, *lda);
}

void CUBLAS_ZGERU (const int *m, const int *n, const cuDoubleComplex *alpha, 
                  const devptr_t *devPtrx, const int *incx,
                  const devptr_t *devPtry, const int *incy,
                  const devptr_t *devPtrA, const int *lda)
{
    cuDoubleComplex *A = (cuDoubleComplex *)(uintptr_t)(*devPtrA);
    cuDoubleComplex *x = (cuDoubleComplex *)(uintptr_t)(*devPtrx);
    cuDoubleComplex *y = (cuDoubleComplex *)(uintptr_t)(*devPtry);    
    cublasZgeru (*m, *n, *alpha, x, *incx, y, *incy, A, *lda);
}

void CUBLAS_ZGERC (const int *m, const int *n, const cuDoubleComplex *alpha, 
                  const devptr_t *devPtrx, const int *incx,
                  const devptr_t *devPtry, const int *incy,
                  const devptr_t *devPtrA, const int *lda)
{
    cuDoubleComplex *A = (cuDoubleComplex *)(uintptr_t)(*devPtrA);
    cuDoubleComplex *x = (cuDoubleComplex *)(uintptr_t)(*devPtrx);
    cuDoubleComplex *y = (cuDoubleComplex *)(uintptr_t)(*devPtry);    
    cublasZgerc (*m, *n, *alpha, x, *incx, y, *incy, A, *lda);
}
/*---------------------------------------------------------------------------*/
/*---------------------------------- BLAS3 ----------------------------------*/
/*---------------------------------------------------------------------------*/

void CUBLAS_SGEMM (const char *transa, const char *transb, const int *m,
                   const int *n, const int *k, const float *alpha,
                   const devptr_t *devPtrA, const int *lda, 
                   const devptr_t *devPtrB, const int *ldb, const float *beta,
                   const devptr_t *devPtrC, const int *ldc)
{
    float *A = (float *)(uintptr_t)(*devPtrA);
    float *B = (float *)(uintptr_t)(*devPtrB);
    float *C = (float *)(uintptr_t)(*devPtrC);
    cublasSgemm (transa[0], transb[0], *m, *n, *k, *alpha, A, *lda, 
                 B, *ldb, *beta, C, *ldc);
}

void CUBLAS_SSYMM (const char *side, const char *uplo, const int *m, 
                   const int *n, const float *alpha, const devptr_t *devPtrA,
                   const int *lda, const devptr_t *devPtrB, const int *ldb, 
                   const float *beta, const devptr_t *devPtrC, const int *ldc)
{
    float *A = (float *)(uintptr_t)(*devPtrA);
    float *B = (float *)(uintptr_t)(*devPtrB);
    float *C = (float *)(uintptr_t)(*devPtrC);
    cublasSsymm (*side, *uplo, *m, *n, *alpha, A, *lda, B, *ldb, *beta, C,
                 *ldc);
}

void CUBLAS_SSYR2K (const char *uplo, const char *trans, const int *n,
                    const int *k, const float *alpha, const devptr_t *devPtrA,
                    const int *lda, const devptr_t *devPtrB, const int *ldb, 
                    const float *beta, const devptr_t *devPtrC, const int *ldc)
{
    float *A = (float *)(uintptr_t)(*devPtrA);
    float *B = (float *)(uintptr_t)(*devPtrB);
    float *C = (float *)(uintptr_t)(*devPtrC);
    cublasSsyr2k (*uplo, *trans, *n, *k, *alpha, A, *lda, B, *ldb, *beta, 
                  C, *ldc);
}

void CUBLAS_SSYRK (const char *uplo, const char *trans, const int *n, 
                   const int *k, const float *alpha, const devptr_t *devPtrA, 
                   const int *lda, const float *beta, const devptr_t *devPtrC,
                   const int *ldc)
{
    float *A = (float *)(uintptr_t)(*devPtrA);
    float *C = (float *)(uintptr_t)(*devPtrC);
    cublasSsyrk (*uplo, *trans, *n, *k, *alpha, A, *lda, *beta, C, *ldc);
}

void CUBLAS_STRMM (const char *side, const char *uplo, const char *transa,
                   const char *diag, const int *m, const int *n,
                   const float *alpha, const devptr_t *devPtrA, const int *lda,
                   const devptr_t *devPtrB, const int *ldb)
{
    float *A = (float *)(uintptr_t)(*devPtrA);
    float *B = (float *)(uintptr_t)(*devPtrB);
    cublasStrmm (*side, *uplo, *transa, *diag, *m, *n, *alpha, A, *lda, B,
                 *ldb);
}

void CUBLAS_STRSM (const char *side, const char *uplo, const char *transa,
                   const char *diag, const int *m, const int *n, 
                   const float *alpha, const devptr_t *devPtrA, const int *lda,
                   const devptr_t *devPtrB, const int *ldb)
{
    float *A = (float *)(uintptr_t)*devPtrA;
    float *B = (float *)(uintptr_t)*devPtrB;
    cublasStrsm (side[0], uplo[0], transa[0], diag[0], *m, *n, *alpha,
                 A, *lda, B, *ldb);
}

void CUBLAS_CGEMM (const char *transa, const char *transb, const int *m,
                   const int *n, const int *k, const cuComplex *alpha,
                   const devptr_t *devPtrA, const int *lda,
                   const devptr_t *devPtrB, const int *ldb, 
                   const cuComplex *beta, const devptr_t *devPtrC,
                   const int *ldc)
{
    cuComplex *A = (cuComplex *)(uintptr_t)*devPtrA;
    cuComplex *B = (cuComplex *)(uintptr_t)*devPtrB;
    cuComplex *C = (cuComplex *)(uintptr_t)*devPtrC;    
    cublasCgemm (transa[0], transb[0], *m, *n, *k, *alpha, A, *lda, B, *ldb, 
                 *beta, C, *ldc);
}


void CUBLAS_CSYMM (const char *side, const char *uplo, const int *m, 
                   const int *n, const cuComplex *alpha, const devptr_t *devPtrA,
                   const int *lda, const devptr_t *devPtrB, const int *ldb, 
                   const cuComplex *beta, const devptr_t *devPtrC, const int *ldc)
{
    cuComplex *A = (cuComplex *)(uintptr_t)(*devPtrA);
    cuComplex *B = (cuComplex *)(uintptr_t)(*devPtrB);
    cuComplex *C = (cuComplex *)(uintptr_t)(*devPtrC);
    cublasCsymm (*side, *uplo, *m, *n, *alpha, A, *lda, B, *ldb, *beta, C,
                 *ldc);
}

void CUBLAS_CHEMM (const char *side, const char *uplo, const int *m, 
                   const int *n, const cuComplex *alpha, const devptr_t *devPtrA,
                   const int *lda, const devptr_t *devPtrB, const int *ldb, 
                   const cuComplex *beta, const devptr_t *devPtrC, const int *ldc)
{
    cuComplex *A = (cuComplex *)(uintptr_t)(*devPtrA);
    cuComplex *B = (cuComplex *)(uintptr_t)(*devPtrB);
    cuComplex *C = (cuComplex *)(uintptr_t)(*devPtrC);
    cublasChemm (*side, *uplo, *m, *n, *alpha, A, *lda, B, *ldb, *beta, C,
                 *ldc);
}

void CUBLAS_CTRMM (const char *side, const char *uplo, const char *transa,
                   const char *diag, const int *m, const int *n,
                   const cuComplex *alpha, const devptr_t *devPtrA, const int *lda,
                   const devptr_t *devPtrB, const int *ldb)
{
    cuComplex *A = (cuComplex *)(uintptr_t)(*devPtrA);
    cuComplex *B = (cuComplex *)(uintptr_t)(*devPtrB);
    cublasCtrmm (*side, *uplo, *transa, *diag, *m, *n, *alpha, A, *lda, B,
                 *ldb);
}

void CUBLAS_CTRSM ( const char *side, const char *uplo, const char *transa, 
                    const char *diag, const int *m, const int *n,
                    const cuComplex *alpha, const devptr_t *devPtrA, const int *lda,
                    const devptr_t *devPtrB, const int *ldb)
{
    cuComplex *A = (cuComplex *)(uintptr_t)*devPtrA;
    cuComplex *B = (cuComplex *)(uintptr_t)*devPtrB;
    cublasCtrsm (side[0], uplo[0], transa[0], diag[0], *m, *n, *alpha,
                 A, *lda, B, *ldb);
}

void CUBLAS_CSYRK (const char *uplo, const char *trans, const int *n, 
                   const int *k, const cuComplex *alpha, const devptr_t *devPtrA, 
                   const int *lda, const cuComplex *beta, const devptr_t *devPtrC,
                   const int *ldc)
{
    cuComplex *A = (cuComplex *)(uintptr_t)(*devPtrA);
    cuComplex *C = (cuComplex *)(uintptr_t)(*devPtrC);
    cublasCsyrk (*uplo, *trans, *n, *k, *alpha, A, *lda, *beta, C, *ldc);
}

void CUBLAS_CSYR2K (const char *uplo, const char *trans, const int *n,
                    const int *k, const cuComplex *alpha, const devptr_t *devPtrA,
                    const int *lda, const devptr_t *devPtrB, const int *ldb, 
                    const cuComplex *beta, const devptr_t *devPtrC,
                    const int *ldc)
{
    cuComplex *A = (cuComplex *)(uintptr_t)(*devPtrA);
    cuComplex *B = (cuComplex *)(uintptr_t)(*devPtrB);
    cuComplex *C = (cuComplex *)(uintptr_t)(*devPtrC);
    cublasCsyr2k (*uplo, *trans, *n, *k, *alpha, A, *lda, B, *ldb, *beta, 
                  C, *ldc);
}

void CUBLAS_CHERK (const char *uplo, const char *trans, const int *n, 
                   const int *k, const float *alpha, const devptr_t *devPtrA, 
                   const int *lda, const float *beta, const devptr_t *devPtrC,
                   const int *ldc)
{
    cuComplex *A = (cuComplex *)(uintptr_t)(*devPtrA);
    cuComplex *C = (cuComplex *)(uintptr_t)(*devPtrC);
    cublasCherk (*uplo, *trans, *n, *k, *alpha, A, *lda, *beta, C, *ldc);
}

void CUBLAS_CHER2K (const char *uplo, const char *trans, const int *n,
                    const int *k, const cuComplex *alpha, const devptr_t *devPtrA,
                    const int *lda, const devptr_t *devPtrB, const int *ldb, 
                    const float *beta, const devptr_t *devPtrC,
                    const int *ldc)
{
    cuComplex *A = (cuComplex *)(uintptr_t)(*devPtrA);
    cuComplex *B = (cuComplex *)(uintptr_t)(*devPtrB);
    cuComplex *C = (cuComplex *)(uintptr_t)(*devPtrC);
    cublasCher2k (*uplo, *trans, *n, *k, *alpha, A, *lda, B, *ldb, *beta, 
                  C, *ldc);
}

void CUBLAS_DGEMM (const char *transa, const char *transb, const int *m,
                   const int *n, const int *k, const double *alpha,
                   const devptr_t *devPtrA, const int *lda, 
                   const devptr_t *devPtrB, const int *ldb, const double *beta,
                   const devptr_t *devPtrC, const int *ldc)
{
    double *A = (double *)(uintptr_t)(*devPtrA);
    double *B = (double *)(uintptr_t)(*devPtrB);
    double *C = (double *)(uintptr_t)(*devPtrC);
    cublasDgemm (transa[0], transb[0], *m, *n, *k, *alpha, A, *lda, 
                 B, *ldb, *beta, C, *ldc);
}

void CUBLAS_DSYMM (const char *side, const char *uplo, const int *m, 
                   const int *n, const double *alpha, const devptr_t *devPtrA,
                   const int *lda, const devptr_t *devPtrB, const int *ldb, 
                   const double *beta, const devptr_t *devPtrC, const int *ldc)
{
    double *A = (double *)(uintptr_t)(*devPtrA);
    double *B = (double *)(uintptr_t)(*devPtrB);
    double *C = (double *)(uintptr_t)(*devPtrC);
    cublasDsymm (*side, *uplo, *m, *n, *alpha, A, *lda, B, *ldb, *beta, C,
                 *ldc);
}

void CUBLAS_DSYR2K (const char *uplo, const char *trans, const int *n,
                    const int *k, const double *alpha, const devptr_t *devPtrA,
                    const int *lda, const devptr_t *devPtrB, const int *ldb, 
                    const double *beta, const devptr_t *devPtrC,
                    const int *ldc)
{
    double *A = (double *)(uintptr_t)(*devPtrA);
    double *B = (double *)(uintptr_t)(*devPtrB);
    double *C = (double *)(uintptr_t)(*devPtrC);
    cublasDsyr2k (*uplo, *trans, *n, *k, *alpha, A, *lda, B, *ldb, *beta, 
                  C, *ldc);
}

void CUBLAS_DSYRK (const char *uplo, const char *trans, const int *n, 
                   const int *k, const double *alpha, const devptr_t *devPtrA, 
                   const int *lda, const double *beta, const devptr_t *devPtrC,
                   const int *ldc)
{
    double *A = (double *)(uintptr_t)(*devPtrA);
    double *C = (double *)(uintptr_t)(*devPtrC);
    cublasDsyrk (*uplo, *trans, *n, *k, *alpha, A, *lda, *beta, C, *ldc);
}

void CUBLAS_ZSYRK (const char *uplo, const char *trans, const int *n, 
                   const int *k, const cuDoubleComplex *alpha, const devptr_t *devPtrA, 
                   const int *lda, const cuDoubleComplex *beta, const devptr_t *devPtrC,
                   const int *ldc)
{
    cuDoubleComplex *A = (cuDoubleComplex *)(uintptr_t)(*devPtrA);
    cuDoubleComplex *C = (cuDoubleComplex *)(uintptr_t)(*devPtrC);
    cublasZsyrk (*uplo, *trans, *n, *k, *alpha, A, *lda, *beta, C, *ldc);
}

void CUBLAS_ZSYR2K (const char *uplo, const char *trans, const int *n,
                    const int *k, const cuDoubleComplex *alpha, const devptr_t *devPtrA,
                    const int *lda, const devptr_t *devPtrB, const int *ldb, 
                    const cuDoubleComplex *beta, const devptr_t *devPtrC,
                    const int *ldc)
{
    cuDoubleComplex *A = (cuDoubleComplex *)(uintptr_t)(*devPtrA);
    cuDoubleComplex *B = (cuDoubleComplex *)(uintptr_t)(*devPtrB);
    cuDoubleComplex *C = (cuDoubleComplex *)(uintptr_t)(*devPtrC);
    cublasZsyr2k (*uplo, *trans, *n, *k, *alpha, A, *lda, B, *ldb, *beta, 
                  C, *ldc);
}

void CUBLAS_DTRMM (const char *side, const char *uplo, const char *transa,
                   const char *diag, const int *m, const int *n,
                   const double *alpha, const devptr_t *devPtrA, 
                   const int *lda, const devptr_t *devPtrB, const int *ldb)
{
    double *A = (double *)(uintptr_t)(*devPtrA);
    double *B = (double *)(uintptr_t)(*devPtrB);
    cublasDtrmm (*side, *uplo, *transa, *diag, *m, *n, *alpha, A, *lda, B,
                 *ldb);
}

void CUBLAS_ZTRMM (const char *side, const char *uplo, const char *transa,
                   const char *diag, const int *m, const int *n,
                   const cuDoubleComplex *alpha, const devptr_t *devPtrA, 
                   const int *lda, const devptr_t *devPtrB, const int *ldb)
{
    cuDoubleComplex *A = (cuDoubleComplex *)(uintptr_t)(*devPtrA);
    cuDoubleComplex *B = (cuDoubleComplex *)(uintptr_t)(*devPtrB);
    cublasZtrmm (*side, *uplo, *transa, *diag, *m, *n, *alpha, A, *lda, B,
                 *ldb);
}


void CUBLAS_DTRSM (const char *side, const char *uplo, const char *transa,
                   const char *diag, const int *m, const int *n, 
                   const double *alpha, const devptr_t *devPtrA,
                   const int *lda, const devptr_t *devPtrB, const int *ldb)
{
    double *A = (double *)(uintptr_t)*devPtrA;
    double *B = (double *)(uintptr_t)*devPtrB;
    cublasDtrsm (side[0], uplo[0], transa[0], diag[0], *m, *n, *alpha,
                 A, *lda, B, *ldb);
}

void CUBLAS_ZTRSM (const char *side, const char *uplo, const char *transa,
                   const char *diag, const int *m, const int *n, 
                   const cuDoubleComplex *alpha, const devptr_t *devPtrA,
                   const int *lda, const devptr_t *devPtrB, const int *ldb)
{
    cuDoubleComplex *A = (cuDoubleComplex *)(uintptr_t)*devPtrA;
    cuDoubleComplex *B = (cuDoubleComplex *)(uintptr_t)*devPtrB;
    cublasZtrsm (side[0], uplo[0], transa[0], diag[0], *m, *n, *alpha,
                 A, *lda, B, *ldb);
}

void CUBLAS_ZGEMM (const char *transa, const char *transb, const int *m,
                   const int *n, const int *k, const cuDoubleComplex *alpha,
                   const devptr_t *devPtrA, const int *lda,
                   const devptr_t *devPtrB, const int *ldb, 
                   const cuDoubleComplex *beta, const devptr_t *devPtrC,
                   const int *ldc)
{
    cuDoubleComplex *A = (cuDoubleComplex *)(uintptr_t)*devPtrA;
    cuDoubleComplex *B = (cuDoubleComplex *)(uintptr_t)*devPtrB;
    cuDoubleComplex *C = (cuDoubleComplex *)(uintptr_t)*devPtrC;    
    cublasZgemm (transa[0], transb[0], *m, *n, *k, *alpha, A, *lda, B, *ldb, 
                 *beta, C, *ldc);
}


void CUBLAS_ZHERK (const char *uplo, const char *trans, const int *n, 
                   const int *k, const double *alpha, const devptr_t *devPtrA, 
                   const int *lda, const double *beta, const devptr_t *devPtrC,
                   const int *ldc)
{
    cuDoubleComplex *A = (cuDoubleComplex *)(uintptr_t)(*devPtrA);
    cuDoubleComplex *C = (cuDoubleComplex *)(uintptr_t)(*devPtrC);
    cublasZherk (*uplo, *trans, *n, *k, *alpha, A, *lda, *beta, C, *ldc);
}

void CUBLAS_ZHER2K (const char *uplo, const char *trans, const int *n,
                    const int *k, const cuDoubleComplex *alpha, const devptr_t *devPtrA,
                    const int *lda, const devptr_t *devPtrB, const int *ldb, 
                    const double *beta, const devptr_t *devPtrC,
                    const int *ldc)
{
    cuDoubleComplex *A = (cuDoubleComplex *)(uintptr_t)(*devPtrA);
    cuDoubleComplex *B = (cuDoubleComplex *)(uintptr_t)(*devPtrB);
    cuDoubleComplex *C = (cuDoubleComplex *)(uintptr_t)(*devPtrC);
    cublasZher2k (*uplo, *trans, *n, *k, *alpha, A, *lda, B, *ldb, *beta, 
                  C, *ldc);
}


void CUBLAS_ZSYMM (const char *side, const char *uplo, const int *m, 
                   const int *n, const cuDoubleComplex *alpha, const devptr_t *devPtrA,
                   const int *lda, const devptr_t *devPtrB, const int *ldb, 
                   const cuDoubleComplex *beta, const devptr_t *devPtrC, const int *ldc)
{
    cuDoubleComplex *A = (cuDoubleComplex *)(uintptr_t)(*devPtrA);
    cuDoubleComplex *B = (cuDoubleComplex *)(uintptr_t)(*devPtrB);
    cuDoubleComplex *C = (cuDoubleComplex *)(uintptr_t)(*devPtrC);
    cublasZsymm (*side, *uplo, *m, *n, *alpha, A, *lda, B, *ldb, *beta, C,
                 *ldc);
}

void CUBLAS_ZHEMM (const char *side, const char *uplo, const int *m, 
                   const int *n, const cuDoubleComplex *alpha, const devptr_t *devPtrA,
                   const int *lda, const devptr_t *devPtrB, const int *ldb, 
                   const cuDoubleComplex *beta, const devptr_t *devPtrC, const int *ldc)
{
    cuDoubleComplex *A = (cuDoubleComplex *)(uintptr_t)(*devPtrA);
    cuDoubleComplex *B = (cuDoubleComplex *)(uintptr_t)(*devPtrB);
    cuDoubleComplex *C = (cuDoubleComplex *)(uintptr_t)(*devPtrC);
    cublasZhemm (*side, *uplo, *m, *n, *alpha, A, *lda, B, *ldb, *beta, C,
                 *ldc);
}


