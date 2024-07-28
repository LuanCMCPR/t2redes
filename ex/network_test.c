#include <stdio.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <string.h>
#include <unistd.h>
#include "network.h"

void create_players(Player *players, int num_players) {
    // Set up players with mock addresses and ports
    for (int i = 0; i < num_players; i++) {
        char ip[16];
        sprintf(ip, "127.0.0.%d", i + 1);
        players[i].id = i;
        players[i].address.sin_family = AF_INET;
        players[i].address.sin_addr.s_addr = inet_addr(ip);
        players[i].address.sin_port = htons(5000 + i);
    }
}

void test_network() {
    const int num_players = 2;
    Player players[num_players];

    // Create mock players
    create_players(players, num_players);

    // Initialize network for both players
    Network net1;

    printf("Initializing network for player 1...\n");
    init_network(&net1, players, num_players);
    printf("Network for player 1 initialized.\n");

    printf("Initializing network for player 2...\n");
    init_network(&net1, players, num_players);
    printf("Network for player 2 initialized.\n");

    // Simulate network initialization process
    printf("Starting network initialization process for player 1...\n");
    init_network_process(&net1);
    printf("Network initialization process for player 1 completed.\n");

    printf("Starting network initialization process for player 2...\n");
    init_network_process(&net1);
    printf("Network initialization process for player 2 completed.\n");

    // Simulate sending and receiving a packet from player 1 to player 2
    printf("Player 1 sending packet to player 2...\n");
    send_packet(&net1, ACTION_INIT_NETWORK, players[1].address.sin_addr.s_addr, 0);
    printf("Packet sent from player 1 to player 2.\n");

    printf("Player 2 receiving packet...\n");
    Packet received_packet = receive_packet(&net1);
    if (received_packet.valid) {
        printf("Packet received by player 2.\n");
    } else {
        printf("Failed to receive packet by player 2.\n");
    }
}

int main() {
    test_network();
    return 0;
}
