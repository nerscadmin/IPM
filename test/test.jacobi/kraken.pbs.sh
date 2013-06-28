#!/bin/csh
#PBS -A TG-ASC070021N
#PBS -l walltime=00:30:00,size=64
#PBS -j oe
#PBS -N jacobi
#PBS -o jacobi-$PBS_JOBID.out

set echo
cd $PBS_O_WORKDIR

time aprun -n 64 ./jacobi 6400 100 8 8 
