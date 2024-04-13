#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/time.h>

#define PORT_OUT1 4001
#define BUFFER_SIZE 64

long long current_time_ms() {
    struct timeval te; 
    gettimeofday(&te, NULL); // Get current time
    long long milliseconds = te.tv_sec * 1000LL + te.tv_usec / 1000; // Calculate milliseconds
    return milliseconds;
}

int main() {
    int clientSocket;
    struct sockaddr_in serverAddr;
    char buffer[BUFFER_SIZE];
    int bytesReceived;
    long long timestamp_ms;

    // Create the socket
    clientSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (clientSocket < 0) {
        perror("ERROR opening socket");
        exit(1);
    }

    // Setup the server address struct
    memset(&serverAddr, 0, sizeof(serverAddr));
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(PORT_OUT1);
    serverAddr.sin_addr.s_addr = inet_addr("127.0.0.1");

    // Connect to the server
    if (connect(clientSocket, (struct sockaddr *)&serverAddr, sizeof(serverAddr)) < 0) {
        perror("ERROR connecting to socket");
        close(clientSocket);
        exit(1);
    }

    // Receive data from the server
    while ((bytesReceived = recv(clientSocket, buffer, BUFFER_SIZE - 1, 0)) > 0) {
        // Null-terminate the buffer, and remove trailing newline
        if (buffer[bytesReceived - 1] == '\n') {
            buffer[bytesReceived - 1] = '\0';
        } else {
            buffer[bytesReceived] = '\0';
        }
        timestamp_ms = current_time_ms();
        printf("{\"timestamp\": %lld, \"out1\": \"%s\"}\n", timestamp_ms, buffer);
    }

    if (bytesReceived < 0) {
        perror("ERROR reading from socket");
    }

    // Close the socket
    close(clientSocket);
    return 0;
}
