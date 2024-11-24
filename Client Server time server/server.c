#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <time.h>

#define PORT 8080
#define BUFFER_SIZE 1024

// Function to get the current CPU load
float get_cpu_load() {
    FILE *file = fopen("/proc/stat", "r");
    if (!file) {
        perror("Could not open /proc/stat");
        return -1.0;
    }

    char line[BUFFER_SIZE];
    long idle, total, prev_idle = 0, prev_total = 0;

    fgets(line, sizeof(line), file);
    fclose(file);

    long user, nice, system, idle_cpu, iowait, irq, softirq, steal;
    sscanf(line, "cpu %ld %ld %ld %ld %ld %ld %ld %ld",
           &user, &nice, &system, &idle_cpu, &iowait, &irq, &softirq, &steal);

    idle = idle_cpu + iowait;
    total = user + nice + system + idle + irq + softirq + steal;

    float cpu_load = 100.0 - ((float)(idle - prev_idle) / (total - prev_total) * 100.0);
    prev_idle = idle;
    prev_total = total;

    return cpu_load;
}

int main() {
    int server_socket, client_socket;
    struct sockaddr_in server_addr, client_addr;
    socklen_t addr_len = sizeof(client_addr);
    char buffer[BUFFER_SIZE];
    time_t raw_time;
    struct tm *time_info;

    // Create server socket
    if ((server_socket = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }

    // Configure server address
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(PORT);

    // Bind the socket
    if (bind(server_socket, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("Bind failed");
        close(server_socket);
        exit(EXIT_FAILURE);
    }

    // Listen for incoming connections
    if (listen(server_socket, 5) < 0) {
        perror("Listen failed");
        close(server_socket);
        exit(EXIT_FAILURE);
    }

    printf("Server listening on port %d...\n", PORT);

    while (1) {
        // Accept client connection
        if ((client_socket = accept(server_socket, (struct sockaddr *)&client_addr, &addr_len)) < 0) {
            perror("Accept failed");
            continue;
        }

        printf("Client connected\n");

        // Get current date and time
        time(&raw_time);
        time_info = localtime(&raw_time);
        strftime(buffer, sizeof(buffer), "Date and Time: %Y-%m-%d %H:%M:%S\n", time_info);

        // Get CPU load
        float cpu_load = get_cpu_load();
        char cpu_load_info[BUFFER_SIZE];
        snprintf(cpu_load_info, sizeof(cpu_load_info), "CPU Load: %.2f%%\n", cpu_load);

        // Send date, time, and CPU load to the client
        strcat(buffer, cpu_load_info);
        send(client_socket, buffer, strlen(buffer), 0);

        printf("Sent to client:\n%s", buffer);

        close(client_socket);
    }

    close(server_socket);
    return 0;
}
