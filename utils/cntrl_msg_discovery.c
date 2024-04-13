#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/time.h>

#define PORT_CONTROL_UDP 4000
#define IP_ADDR "127.0.0.1"

// I used this program to figure out the control message values for the 
// property (+associated value) fields as they were not documented in the instructions.
// Was this planned or did I miss something?
// I did not include this in the makefile, build this with "gcc cntrl_msg_discovery.c -o cntrl_msg_discovery"

struct message {
    uint16_t operation;     // 1 = read, 2 = write
    uint16_t object;        // 1 = out1, 2 = out2, 3 = out3
    uint16_t property;      // ?
    uint16_t value;         // ?
};

int udp_sockfd;
struct sockaddr_in udp_server_addr;

void send_udp_message(struct message msg) {
    if (sendto(udp_sockfd, &msg, sizeof(msg), 0, (const struct sockaddr *)&udp_server_addr, sizeof(udp_server_addr)) < 0) {
        perror("UDP Control message sending failed");
        close(udp_sockfd);
        exit(1);
    }
}

int main() {
    if ((udp_sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        perror("ERROR opening UDP socket");
        exit(EXIT_FAILURE);
    }

    memset(&udp_server_addr, 0, sizeof(udp_server_addr));
    udp_server_addr.sin_family = AF_INET;
    udp_server_addr.sin_port = htons(PORT_CONTROL_UDP);
    udp_server_addr.sin_addr.s_addr = inet_addr(IP_ADDR);

    // Go over the potential msg property fields
    // The server will output errors based on msg content
    //      For wrong property field -> error,no such property
    //      For OK property field and bad value -> error,Value out of range, 65535 not in [0, 1]
    for (int i=0; i <= 1000; ++i) {
        struct message msg = {htons(2), htons(1), htons(i), htons(-1)};
        send_udp_message(msg);
        usleep(100000);     // 100ms sleep
    }
}
