#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#define PORT 8080
#define BUFFER_SIZE 1024
#define MAX_CLIENTS 10

typedef struct {
    int node_no;
    char ip_address[INET_ADDRSTRLEN];
    int port_no;
} ClientInfo;

int main() {
    int client_socket;
    struct sockaddr_in server_addr;
    ClientInfo master_table[MAX_CLIENTS];

    // Create client socket
    if ((client_socket = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }

    // Configure server address
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);

    // Convert IP address to binary form
    if (inet_pton(AF_INET, "127.0.0.1", &server_addr.sin_addr) <= 0) {
        perror("Invalid address or address not supported");
        close(client_socket);
        exit(EXIT_FAILURE);
    }

    // Connect to the server
    if (connect(client_socket, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("Connection failed");
        close(client_socket);
        exit(EXIT_FAILURE);
    }

    printf("Connected to server on port %d...\n", PORT);

    // Receive updated table from the server
    int bytes_read = read(client_socket, master_table, sizeof(master_table));
    if (bytes_read > 0) {
        printf("Updated Master Table received from server:\n");
        printf("Node No.\tIP Address\t\tPort No.\n");
        printf("------------------------------------------------\n");
        for (int i = 0; i < MAX_CLIENTS; i++) {
            if (master_table[i].node_no != 0) {
                printf("%d\t\t%s\t\t%d\n",
                       master_table[i].node_no,
                       master_table[i].ip_address,
                       master_table[i].port_no);
            }
        }
    } else {
        printf("Failed to receive updated table from server\n");
    }

    close(client_socket);
    return 0;
}
