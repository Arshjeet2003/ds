#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#define PORT 8080
#define BUFFER_SIZE 1024

// Function to get CPU load (Linux-specific)
float get_cpu_load() {
    FILE *fp = fopen("/proc/stat", "r");
    if (!fp) {
        perror("Failed to open /proc/stat");
        exit(EXIT_FAILURE);
    }

    char buffer[BUFFER_SIZE];
    unsigned long long int user, nice, system, idle, iowait, irq, softirq, steal;
    fgets(buffer, sizeof(buffer), fp); // Read the first line
    sscanf(buffer, "cpu  %llu %llu %llu %llu %llu %llu %llu %llu",
           &user, &nice, &system, &idle, &iowait, &irq, &softirq, &steal);
    fclose(fp);

    // Calculate CPU load percentage
    unsigned long long int idle_time = idle + iowait;
    unsigned long long int total_time = user + nice + system + idle_time + irq + softirq + steal;
    static unsigned long long int prev_idle = 0, prev_total = 0;

    unsigned long long int delta_idle = idle_time - prev_idle;
    unsigned long long int delta_total = total_time - prev_total;

    prev_idle = idle_time;
    prev_total = total_time;

    return (1.0 - (float)delta_idle / delta_total) * 100;
}

int main() {
    int server_fd, new_socket;
    struct sockaddr_in address;
    int opt = 1;
    int addrlen = sizeof(address);
    char buffer[BUFFER_SIZE] = {0};

    // Create socket
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("Socket failed");
        exit(EXIT_FAILURE);
    }

    // Attach socket to port
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt))) {
        perror("Setsockopt failed");
        exit(EXIT_FAILURE);
    }

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    // Bind socket to address
    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        perror("Bind failed");
        exit(EXIT_FAILURE);
    }

    // Listen for connections
    if (listen(server_fd, 3) < 0) {
        perror("Listen failed");
        exit(EXIT_FAILURE);
    }

    printf("Node B (Server) listening on port %d...\n", PORT);

    while (1) {
        // Accept connection
        if ((new_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t *)&addrlen)) < 0) {
            perror("Accept failed");
            exit(EXIT_FAILURE);
        }

        printf("Connection established with Node A\n");

        // Calculate CPU load
        float cpu_load = get_cpu_load();

        // Send CPU load to Node A
        snprintf(buffer, BUFFER_SIZE, "CPU Load: %.2f%%", cpu_load);
        send(new_socket, buffer, strlen(buffer), 0);

        printf("Sent CPU load to Node A: %s\n", buffer);

        close(new_socket);
    }

    return 0;
}
