/*
Here is an implementation of a Distributed Application using MPI for remote computation. The program demonstrates how multiple processes communicate to perform a computational task using Message Passing Interface (MPI).
Problem Statement

Design a distributed application where:

    A master process divides a computational task among multiple worker processes.
    Each worker computes its part of the task (e.g., a sum of a segment of an array).
    The workers send results back to the master, which aggregates them to get the final result.

sudo apt update
sudo apt install openmpi-bin libopenmpi-dev

Check Compiler Include Path

    Ensure the compiler knows where to find the mpi.h file.
    For OpenMPI, the mpi.h file is usually located in /usr/include or /usr/local/include.
    To explicitly provide the include path, pass the -I flag to the compiler:

    mpicc -I/usr/include -o mpi_program mpi_program.c

3. Verify Installation

    Check if mpi.h exists:

find /usr -name "mpi.h"

If mpi.h exists in a directory (e.g., /usr/local/include), include it during compilation:

mpicc -I/usr/local/include -o mpi_program mpi_program.c

*/

#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>

#define ARRAY_SIZE 100 // Size of the array
#define MASTER 0       // Master process rank

// Function to compute the sum of an array segment
int compute_sum(int *array, int start, int end) {
    int sum = 0;
    for (int i = start; i < end; i++) {
        sum += array[i];
    }
    return sum;
}

int main(int argc, char *argv[]) {
    int rank, size;
    int array[ARRAY_SIZE];
    int local_sum = 0, global_sum = 0;

    // Initialize MPI environment
    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    int elements_per_process = ARRAY_SIZE / size;

    // Initialize the array in the master process
    if (rank == MASTER) {
        printf("Master: Initializing array...\n");
        for (int i = 0; i < ARRAY_SIZE; i++) {
            array[i] = i + 1; // Initialize array with values 1 to 100
        }
    }

    // Scatter array segments to all processes
    int *sub_array = (int *)malloc(elements_per_process * sizeof(int));
    MPI_Scatter(array, elements_per_process, MPI_INT, sub_array, elements_per_process, MPI_INT, MASTER, MPI_COMM_WORLD);

    // Each process computes the sum of its segment
    local_sum = compute_sum(sub_array, 0, elements_per_process);
    printf("Process %d: Local sum = %d\n", rank, local_sum);

    // Gather all local sums to the master process
    MPI_Reduce(&local_sum, &global_sum, 1, MPI_INT, MPI_SUM, MASTER, MPI_COMM_WORLD);

    // Master process prints the global sum
    if (rank == MASTER) {
        printf("Master: Global sum = %d\n", global_sum);
    }

    // Free dynamically allocated memory and finalize MPI environment
    free(sub_array);
    MPI_Finalize();
    return 0;
}