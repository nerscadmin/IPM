#!/bin/bash
# specify the queue: lr_debug, lr_batch 
#PBS -q lr_regular
# specify the account your job will be charged to 
##PBS -A 
# specify number of nodes, processors per node and node property
#PBS -l nodes=4:ppn=4:lr
# specify output for STDERR and STDOUT
#PBS -e job.err
#PBS -o job.out
#executable commands... 
# change to working directory

cd $PBS_O_WORKDIR
mpirun -np 64 ./jacobi.ipm 1000 1000 8 8 
