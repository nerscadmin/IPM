#!/bin/bash
#SBATCH -N 1
#SBATCH -C haswell
#SBATCH -p debug
#SBATCH -t 00:10:00

# This SLURM job script is intended to be run on Cori Phase-1 at
# NERSC.

# Change to the IPM test directory, compile the test programs and then
# submit this script from the same directory, i.e.
# > cd IPM/test
# > make
# > sbatch run-test-suite.sh

# The script runs each of the parallel test programs with 2 MPI ranks
# and generates an IPM XML file with the same name as the test
# program. Success or failure is determined based on the return value
# of each test program. The total number of failures is reported at
# the end of the script. Note that the script does not check IPM
# correctness and is only useful for identifying severe IPM
# problems. Future versions should analyze the data in the IPM XML
# files.

export OMP_NUM_THREADS=1
export OMP_PLACES=threads
export OMP_PROC_BIND=spread
export IPM_REPORT=full
export IPM_LOG=full

ptests[0]="allgather"
ptests[1]="allgatherv"
ptests[2]="allreduce"
ptests[3]="alltoall"
ptests[4]="alltoallv"
ptests[5]="bcast"
ptests[6]="fhello"
ptests[7]="fring"
ptests[8]="gather"
ptests[9]="gatherv"
ptests[10]="hello"
ptests[11]="keyhist"
ptests[12]="pcontrol"
ptests[13]="simple_mpi"
ptests[14]="status_ignore"

fail_count=0
for i in "${ptests[@]}"; do
    app="./test.${i}/${i}.ipm"
    ipm="./${i}.ipm.xml"
    export IPM_OUTFILE="${ipm}"

    echo "About to run ${app}"
    srun -n 2 -c 32 --cpu_bind=cores ${app}
    # mpiexec -n 2 ${app} # Alternative for non-SLURM jobs
    if [ $? -eq 0 ]; then
	echo -e "SUCCESS: ${app}\n"
    else
	echo -e "FAILURE: ${app}\n"
	((fail_count++))
    fi
done

echo "Total failures: ${fail_count}"
