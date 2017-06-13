#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "mpi.h"

int main(int argc, char* argv[]) {
    MPI_Init(&argc, &argv);

    MPI_Pcontrol(1, "all_sleep_calls");
    MPI_Pcontrol(1, "first_2_sleeps");
    MPI_Pcontrol(1, "sleep_5");
    sleep(5);
    MPI_Pcontrol(-1, "sleep_5");
    MPI_Pcontrol(1, "sleep_10");
    sleep(10);
    MPI_Pcontrol(-1, "sleep_10");
    MPI_Pcontrol(-1, "first_2_sleeps");
    MPI_Pcontrol(1, "sleep_15");
    sleep(15);
    MPI_Pcontrol(-1, "sleep_15");
    MPI_Pcontrol(-1, "all_sleep_calls");

    MPI_Finalize();
	return 0;
}

