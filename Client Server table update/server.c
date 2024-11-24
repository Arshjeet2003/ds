#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <pthread.h>

#define PORT 8080
#define MAX_CLIENTS 10
#define BUFFER_SIZE 1024

typedef struct {
    int node_no;
    char ip_address[INET_ADDRSTRLEN];
    int port_no;
} ClientInfo;

ClientInfo master_table[MAX_CLIENTS];
int client_count = 0;
pthread_mutex_t table_mutex;

// Function to handle client connection
void *handle_client(void *arg) {
    int client_socket = *(int *)arg;
    free(arg);

    struct sockaddr_in client_addr;
    socklen_t addr_len = sizeof(client_addr);
    getpeername(client_socket, (struct sockaddr *)&client_addr, &addr_len);

    // Add client to the master table
    pthread_mutex_lock(&table_mutex);
    if (client_count < MAX_CLIENTS) {
        master_table[client_count].node_no = client_count + 1;
        inet_ntop(AF_INET, &client_addr.sin_addr, master_table[client_count].ip_address, INET_ADDRSTRLEN);
        master_table[client_count].port_no = ntohs(client_addr.sin_port);
        client_count++;
    } else {
        printf("Maximum client limit reached. Cannot add more clients.\n");
    }
    pthread_mutex_unlock(&table_mutex);

    // Send updated table to the client
    send(client_socket, master_table, sizeof(master_table), 0);

    printf("Sent updated table to client %s:%d\n",
           master_table[client_count - 1].ip_address,
           master_table[client_count - 1].port_no);

    close(client_socket);
    return NULL;
}

int main() {
    int server_socket, client_socket;
    struct sockaddr_in server_addr, client_addr;
    socklen_t addr_len = sizeof(client_addr);

    // Initialize mutex
    pthread_mutex_init(&table_mutex, NULL);

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

        printf("Client connected: %s:%d\n",
               inet_ntoa(client_addr.sin_addr),
               ntohs(client_addr.sin_port));

        // Allocate memory for the client socket and create a thread
        int *new_socket = malloc(sizeof(int));
        if (!new_socket) {
            perror("Memory allocation failed");
            close(client_socket);
            continue;
        }
        *new_socket = client_socket;

        pthread_t thread_id;
        if (pthread_create(&thread_id, NULL, handle_client, new_socket) != 0) {
            perror("Thread creation failed");
            free(new_socket);
            close(client_socket);
            continue;
        }

        // Detach the thread
        pthread_detach(thread_id);
    }

    close(server_socket);
    pthread_mutex_destroy(&table_mutex);
    return 0;
}
