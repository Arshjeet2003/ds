#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <arpa/inet.h>

#define PORT 8080
#define BUFFER_SIZE 1024

// Function to send messages to the server
void *send_messages(void *socket_desc) {
    int sock = *(int *)socket_desc;
    char message[BUFFER_SIZE];

    while (1) {
        fgets(message, BUFFER_SIZE, stdin);
        send(sock, message, strlen(message), 0);
    }
    return NULL;
}

// Function to receive messages from the server
void *receive_messages(void *socket_desc) {
    int sock = *(int *)socket_desc;
    char buffer[BUFFER_SIZE];
    int bytes_read;

    while ((bytes_read = read(sock, buffer, sizeof(buffer))) > 0) {
        buffer[bytes_read] = '\0';  // Null-terminate the received string
        printf("Server: %s\n", buffer);
    }

    printf("Server disconnected\n");
    close(sock);
    exit(0);
    return NULL;
}

int main() {
    int sock = 0;
    struct sockaddr_in serv_addr;

    // Create client socket
    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("Socket creation error");
        return -1;
    }

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT);

    // Convert address to binary form and connect to the server
    if (inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr) <= 0) {
        perror("Invalid address/Address not supported");
        return -1;
    }

    if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
        perror("Connection failed");
        return -1;
    }

    printf("Connected to the server\n");

    // Create threads for sending and receiving messages
    pthread_t send_thread, receive_thread;

    if (pthread_create(&send_thread, NULL, send_messages, &sock) != 0) {
        perror("Send thread creation failed");
        return -1;
    }

    if (pthread_create(&receive_thread, NULL, receive_messages, &sock) != 0) {
        perror("Receive thread creation failed");
        return -1;
    }

    // Wait for threads to finish (infinite loop until program is terminated)
    pthread_join(send_thread, NULL);
    pthread_join(receive_thread, NULL);

    close(sock);
    return 0;
}
