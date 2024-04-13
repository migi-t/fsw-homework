#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/time.h>
#include <stdbool.h>

#define PORT_CONTROL_UDP 4000
#define PORT_OUT1_TCP 4001
#define PORT_OUT2_TCP 4002
#define PORT_OUT3_TCP 4003
#define NUM_TCP_SOCKETS 3
#define NUM_SOCKETS 4
#define BUFFER_SIZE 64
#define IP_ADDR "127.0.0.1"

struct message {
    uint16_t operation;     // 1 = read, 2 = write
    uint16_t object;        // 1 = out1, 2 = out2, 3 = out3
    uint16_t property;      // Enabled = 14, Ampl = 170, Freq = 255, glitch_chance = 300
    uint16_t value;         // Enabled [0, 1], Ampl [1000, 10000], Freq [50, 2000 (ms ?)], glitch_chance [0, 100 (% ?)]
};

int udp_sockfd;
struct sockaddr_in udp_server_addr;

int create_udp_socket(int port) {
    if ((udp_sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        perror("ERROR opening UDP socket");
        exit(EXIT_FAILURE);
    }

    memset(&udp_server_addr, 0, sizeof(udp_server_addr));
    udp_server_addr.sin_family = AF_INET;
    udp_server_addr.sin_port = htons(port);
    udp_server_addr.sin_addr.s_addr = inet_addr(IP_ADDR);

    return udp_sockfd;
}

void send_udp_message(struct message msg) {
    if (sendto(udp_sockfd, &msg, sizeof(msg), 0, (const struct sockaddr *)&udp_server_addr, sizeof(udp_server_addr)) < 0) {
        perror("UDP Control message sending failed");
        close(udp_sockfd);
        exit(1);
    }
}

int create_tcp_socket(int port) {
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        perror("ERROR opening TCP socket");
        exit(1);
    }

    struct sockaddr_in serverAddr;
    memset(&serverAddr, 0, sizeof(serverAddr));
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(port);
    serverAddr.sin_addr.s_addr = inet_addr(IP_ADDR);

    if (connect(sockfd, (struct sockaddr *)&serverAddr, sizeof(serverAddr)) < 0) {
        perror("ERROR connecting to TCP socket");
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

// Only reacts when the output3 crosses the limit
void process_output3(float val, bool *isOverLimit) {

    // output3 value went over 3.0
    if (*isOverLimit == false && val >= 3.0) {
        *isOverLimit = true;

        // output1 frequency -> 1Hz (i.e., 1s cycle time)
        struct message write_msg_freq = {htons(2), htons(1), htons(255), htons(1000)};
        send_udp_message(write_msg_freq);

        // output1 amplitude -> 8000
        struct message write_msg_ampl = {htons(2), htons(1), htons(170), htons(8000)};
        send_udp_message(write_msg_ampl);

    // output3 value went under 3.0
    } else if (*isOverLimit == true && val < 3.0) {
        *isOverLimit = false;

        // output1 frequency -> 2Hz (i.e., 0.5s cycle time)
        struct message write_msg_freq = {htons(2), htons(1), htons(255), htons(500)};
        send_udp_message(write_msg_freq);

        // output1 amplitude -> 4000
        struct message write_msg_ampl = {htons(2), htons(1), htons(170), htons(4000)};
        send_udp_message(write_msg_ampl);
    }
}

int main() {
    int sockets[NUM_SOCKETS] = {create_tcp_socket(PORT_OUT1_TCP), create_tcp_socket(PORT_OUT2_TCP), create_tcp_socket(PORT_OUT3_TCP), create_udp_socket(PORT_CONTROL_UDP)};
    int maxfd = 0;
    for (int i = 0; i < NUM_TCP_SOCKETS; i++) {
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

    bool isOverLimit;   // 3.0 is the limit to act upon in output3

    struct timeval last_print_time;
    gettimeofday(&last_print_time, NULL);
    while (1) {
        FD_ZERO(&readfds);
        for (int i = 0; i < NUM_TCP_SOCKETS; i++) {
            FD_SET(sockets[i], &readfds);
        }

        // Monitor multiple sockets simultaneously (timeout at 20ms)
        int activity = select(maxfd, &readfds, NULL, NULL, &timeout);

        if (activity < 0) {
            perror("ERROR on select");
            exit(1);
        }

        // Loop over our TCP sockets
        for (int i = 0; i < NUM_TCP_SOCKETS; i++) {

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

                        // i=2 means output3 socket
                        if (i == 2) {
                            process_output3(out_value, &isOverLimit);
                        }
                    }
                }
            }
        }

        // Check if 20ms has elapsed. If yes, print and reset the stored outputs
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
