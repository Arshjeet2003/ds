#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <ctype.h>

#define BUFFER_SIZE 1024
#define SERVER1_PORT 8081
#define SERVER2_PORT 8082
#define SERVER1_IP "127.0.0.1"
#define SERVER2_IP "127.0.0.1"

// Function to get CPU utilization
double get_cpu_utilization() {
    FILE *fp = fopen("/proc/stat", "r");
    if (!fp) {
        perror("Failed to open /proc/stat");
        return 100.0; // Return high utilization in case of failure
    }

    char line[256];
    fgets(line, sizeof(line), fp);
    fclose(fp);

    // Parse CPU statistics
    char cpu_label[5];
    unsigned long user, nice, system, idle;
    sscanf(line, "%s %lu %lu %lu %lu", cpu_label, &user, &nice, &system, &idle);

    // Calculate CPU usage
    static unsigned long prev_user = 0, prev_nice = 0, prev_system = 0, prev_idle = 0;
    unsigned long diff_user = user - prev_user;
    unsigned long diff_nice = nice - prev_nice;
    unsigned long diff_system = system - prev_system;
    unsigned long diff_idle = idle - prev_idle;

    prev_user = user;
    prev_nice = nice;
    prev_system = system;
    prev_idle = idle;

    double usage = (double)(diff_user + diff_nice + diff_system) / 
                   (diff_user + diff_nice + diff_system + diff_idle) * 100;
    return usage;
}

// Function to forward the string to a server
void forward_to_server(const char *server_ip, int server_port, const char *input, char *output) {
    int sock;
    struct sockaddr_in server_addr;

    // Create socket
    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }

    // Configure server address
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(server_port);

    if (inet_pton(AF_INET, server_ip, &server_addr.sin_addr) <= 0) {
        perror("Invalid address");
        close(sock);
        exit(EXIT_FAILURE);
    }

    // Connect to the server
    if (connect(sock, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("Connection to server failed");
        close(sock);
        exit(EXIT_FAILURE);
    }

    // Send the string
    send(sock, input, strlen(input), 0);

    // Receive the response
    memset(output, 0, BUFFER_SIZE);
    read(sock, output, BUFFER_SIZE);

    close(sock);
}

int main() {
    int lb_socket, client_socket;
    struct sockaddr_in lb_addr, client_addr;
    socklen_t addr_len = sizeof(client_addr);
    char buffer[BUFFER_SIZE];
    char response[BUFFER_SIZE];

    // Create load balancer socket
    if ((lb_socket = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }

    // Configure load balancer address
    lb_addr.sin_family = AF_INET;
    lb_addr.sin_addr.s_addr = INADDR_ANY;
    lb_addr.sin_port = htons(8080);

    // Bind the socket
    if (bind(lb_socket, (struct sockaddr *)&lb_addr, sizeof(lb_addr)) < 0) {
        perror("Bind failed");
        close(lb_socket);
        exit(EXIT_FAILURE);
    }

    // Listen for incoming connections
    if (listen(lb_socket, 5) < 0) {
        perror("Listen failed");
        close(lb_socket);
        exit(EXIT_FAILURE);
    }

    printf("Load balancer listening on port 8080...\n");

    while (1) {
        // Accept client connection
        if ((client_socket = accept(lb_socket, (struct sockaddr *)&client_addr, &addr_len)) < 0) {
            perror("Accept failed");
            continue;
        }

        // Read string from client
        memset(buffer, 0, BUFFER_SIZE);
        int bytes_read = read(client_socket, buffer, BUFFER_SIZE);
        if (bytes_read > 0) {
            buffer[bytes_read] = '\0';
            printf("Received: %s\n", buffer);

            // Get CPU utilization of both servers
            double cpu1 = get_cpu_utilization();
            sleep(1); // Simulate delay for accurate calculation
            double cpu2 = get_cpu_utilization();

            // Forward to the server with lower CPU utilization
            if (cpu1 < cpu2) {
                forward_to_server(SERVER1_IP, SERVER1_PORT, buffer, response);
            } else {
                forward_to_server(SERVER2_IP, SERVER2_PORT, buffer, response);
            }

            // Send the response back to the client
            send(client_socket, response, strlen(response), 0);
            printf("Sent: %s\n", response);
        }

        close(client_socket); // Close the connection
    }

    close(lb_socket);
    return 0;
}
