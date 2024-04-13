#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/time.h>

#define PORT_OUT1 4001
#define PORT_OUT2 4002
#define PORT_OUT3 4003
#define NUM_SOCKETS 3
#define BUFFER_SIZE 64

int create_socket(int port) {
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        perror("ERROR opening socket");
        exit(1);
    }

    struct sockaddr_in serverAddr;
    memset(&serverAddr, 0, sizeof(serverAddr));
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(port);
    serverAddr.sin_addr.s_addr = inet_addr("127.0.0.1");

    if (connect(sockfd, (struct sockaddr *)&serverAddr, sizeof(serverAddr)) < 0) {
        perror("ERROR connecting");
        close(sockfd);
        exit(1);
    }

    return sockfd;
}

// Print the output in a JSON like format
void print_output(long long timestamp_ms, char *outputs[]) {
    printf("{\"timestamp\": %lld, \"out1\": \"%s\", \"out2\": \"%s\", \"out3\": \"%s\"}\n",
                            timestamp_ms, outputs[0], outputs[1], outputs[2]);
}

int main() {
    int sockets[NUM_SOCKETS] = {create_socket(PORT_OUT1), create_socket(PORT_OUT2), create_socket(PORT_OUT3)};
    int maxfd = 0;
    for (int i = 0; i < NUM_SOCKETS; i++) {
        maxfd = (sockets[i] > maxfd) ? sockets[i] : maxfd;
    }
    maxfd += 1;

    fd_set readfds;
    char buffer[BUFFER_SIZE];
    char *outputs[3] = {"--", "--", "--"};  // Most recent outputs from sockets are stored here
    float out_value;

    // Set timeout for 20 milliseconds
    struct timeval timeout;
    timeout.tv_sec = 0;
    timeout.tv_usec = 20000;
    struct timeval now;

    struct timeval last_print_time;
    gettimeofday(&last_print_time, NULL);
    while (1) {
        FD_ZERO(&readfds);
        for (int i = 0; i < NUM_SOCKETS; i++) {
            FD_SET(sockets[i], &readfds);
        }

        // Monitor multiple sockets simultaneously (timeout at 100ms)
        int activity = select(maxfd, &readfds, NULL, NULL, &timeout);

        if (activity < 0) {
            perror("ERROR on select");
            exit(1);
        }

        // Loop over our sockets
        for (int i = 0; i < NUM_SOCKETS; i++) {

            // Check if a socket has new events
            if (FD_ISSET(sockets[i], &readfds)) {
                int bytesReceived = recv(sockets[i], buffer, BUFFER_SIZE - 1, 0);

                if (bytesReceived < 0) {
                    perror("ERROR reading from socket");
                } else {
                    // Null terminate the data in buffer
                    buffer[bytesReceived] = '\0';
                    // (Try to) Parse the float value from the buffer into out_value
                    if (sscanf(buffer, "%f", &out_value) == 1) {
                        // Store the parsed value as a string
                        char val_str[16];
                        snprintf(val_str, sizeof(val_str), "%0.1f", out_value);
                        outputs[i] = strdup(val_str);
                    }
                }
            }
        }

        // Check if 100ms has elapsed. If yes, print and reset the stored outputs
        gettimeofday(&now, NULL);
        long long now_ms = now.tv_sec * 1000LL + now.tv_usec / 1000;
        if (now_ms - (last_print_time.tv_sec * 1000LL + last_print_time.tv_usec / 1000) >= 20) {
            print_output(now_ms, outputs);
            last_print_time = now;
            outputs[0] = outputs[1] = outputs[2] = "--";    // Reset output values, chained assigment
        }
    }

    return 0;
}
