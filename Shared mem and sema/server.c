//gcc -o server_shared_memory server.c -lrt -pthread
#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <unistd.h>
#include <semaphore.h>
#include <string.h>
#include <sys/types.h>
#include <errno.h>

#define SHARED_MEM_NAME "/shared_counter"
#define SEMAPHORE_NAME "/sem_counter"
#define SHARED_MEM_SIZE sizeof(int)

int main() {
    // Create or open a shared memory object
    int shm_fd = shm_open(SHARED_MEM_NAME, O_CREAT | O_RDWR, 0666);
    if (shm_fd == -1) {
        perror("shm_open failed");
        exit(EXIT_FAILURE);
    }

    // Set the size of the shared memory object
    if (ftruncate(shm_fd, SHARED_MEM_SIZE) == -1) {
        perror("ftruncate failed");
        shm_unlink(SHARED_MEM_NAME); // Cleanup
        exit(EXIT_FAILURE);
    }

    // Map the shared memory object into the process's address space
    int *shared_counter = mmap(NULL, SHARED_MEM_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
    if (shared_counter == MAP_FAILED) {
        perror("mmap failed");
        shm_unlink(SHARED_MEM_NAME); // Cleanup
        exit(EXIT_FAILURE);
    }

    // Initialize the counter to 0
    *shared_counter = 0;

    // Create or open a named semaphore
    sem_t *sem = sem_open(SEMAPHORE_NAME, O_CREAT, 0666, 1); // Initial value of 1
    if (sem == SEM_FAILED) {
        perror("sem_open failed");
        munmap(shared_counter, SHARED_MEM_SIZE);
        shm_unlink(SHARED_MEM_NAME); // Cleanup
        exit(EXIT_FAILURE);
    }

    printf("Server is running. Shared memory and semaphore initialized.\n");
    printf("Press Ctrl+C to terminate.\n");

    while (1) {
        // Simulate some work by sleeping
        sleep(1);

        // Enter the critical section
        sem_wait(sem);

        // Increment the shared counter
        (*shared_counter)++;
        printf("Counter incremented: %d\n", *shared_counter);

        // Exit the critical section
        sem_post(sem);
    }

    // Cleanup resources (though this won't be reached in the loop)
    munmap(shared_counter, SHARED_MEM_SIZE);
    shm_unlink(SHARED_MEM_NAME);
    sem_close(sem);
    sem_unlink(SEMAPHORE_NAME);

    return 0;
}
