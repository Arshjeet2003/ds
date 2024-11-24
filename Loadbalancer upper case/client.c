#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#define PORT 8080           // Load Balancer Port
#define BUFFER_SIZE 1024    // Maximum buffer size

int main() {
    int client_socket;
    struct sockaddr_in server_addr;
    char buffer[BUFFER_SIZE];
    char response[BUFFER_SIZE];

    // Create socket
    if ((client_socket = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }

    // Configure server address (load balancer address)
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);

    // Convert IP address to binary form
    if (inet_pton(AF_INET, "127.0.0.1", &server_addr.sin_addr) <= 0) {
        perror("Invalid address or address not supported");
        close(client_socket);
        exit(EXIT_FAILURE);
    }

    // Connect to the load balancer
    if (connect(client_socket, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("Connection to load balancer failed");
        close(client_socket);
        exit(EXIT_FAILURE);
    }

    printf("Connected to load balancer on port %d...\n", PORT);

    // Input lowercase string from user
    printf("Enter a lowercase string: ");
    fgets(buffer, BUFFER_SIZE, stdin);
    buffer[strcspn(buffer, "\n")] = '\0'; // Remove trailing newline

    // Send string to the load balancer
    send(client_socket, buffer, strlen(buffer), 0);
    printf("Sent to load balancer: %s\n", buffer);

    // Receive response from the load balancer
    memset(response, 0, BUFFER_SIZE);
    int bytes_read = read(client_socket, response, BUFFER_SIZE);
    if (bytes_read > 0) {
        response[bytes_read] = '\0';
        printf("Response from load balancer: %s\n", response);
    } else {
        printf("Failed to receive response from load balancer\n");
    }

    // Close the socket
    close(client_socket);
    return 0;
}
