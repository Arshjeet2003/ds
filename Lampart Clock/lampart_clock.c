#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define NUM_PROCESSES 3

// Struct to represent a process
typedef struct {
    int lamport_clock;       // Lamport clock for the process
    int vector_clock[NUM_PROCESSES]; // Vector clock for the process
} Process;

// Function to initialize the clocks
void initialize(Process processes[]) {
    for (int i = 0; i < NUM_PROCESSES; i++) {
        processes[i].lamport_clock = 0;
        for (int j = 0; j < NUM_PROCESSES; j++) {
            processes[i].vector_clock[j] = 0;
        }
    }
}

// Function to simulate an event in a process (internal event or message sending)
void event(Process *process, int process_id) {
    printf("Process %d: Internal event\n", process_id);
    process->lamport_clock++;
    process->vector_clock[process_id]++;
}

// Function to simulate a message being sent from one process to another
void send_message(Process *sender, Process *receiver, int sender_id, int receiver_id) {
    printf("Process %d: Sending message to Process %d\n", sender_id, receiver_id);

    // Update Lamport clock
    sender->lamport_clock++;
    receiver->lamport_clock = sender->lamport_clock > receiver->lamport_clock
                                  ? sender->lamport_clock + 1
                                  : receiver->lamport_clock + 1;

    // Update Vector clock
    sender->vector_clock[sender_id]++;
    for (int i = 0; i < NUM_PROCESSES; i++) {
        receiver->vector_clock[i] = sender->vector_clock[i] > receiver->vector_clock[i]
                                        ? sender->vector_clock[i]
                                        : receiver->vector_clock[i];
    }
}

// Function to print the clocks
void print_clocks(Process processes[]) {
    printf("\nCurrent Clocks:\n");
    for (int i = 0; i < NUM_PROCESSES; i++) {
        printf("Process %d - Lamport Clock: %d, Vector Clock: [", i, processes[i].lamport_clock);
        for (int j = 0; j < NUM_PROCESSES; j++) {
            printf("%d", processes[i].vector_clock[j]);
            if (j < NUM_PROCESSES - 1) printf(", ");
        }
        printf("]\n");
    }
    printf("\n");
}

int main() {
    Process processes[NUM_PROCESSES];
    initialize(processes);

    printf("Initial State:\n");
    print_clocks(processes);

    // Simulating events and message exchanges
    event(&processes[0], 0); // Process 0 performs an internal event
    event(&processes[1], 1); // Process 1 performs an internal event
    send_message(&processes[0], &processes[1], 0, 1); // Process 0 sends a message to Process 1
    event(&processes[2], 2); // Process 2 performs an internal event
    send_message(&processes[1], &processes[2], 1, 2); // Process 1 sends a message to Process 2
    send_message(&processes[2], &processes[0], 2, 0); // Process 2 sends a message to Process 0

    printf("Final State:\n");
    print_clocks(processes);

    return 0;
}
