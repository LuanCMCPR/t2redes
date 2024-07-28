#include "network.h"
#include <stdio.h>
#include <stdlib.h>

int main() {
    Player players[2];

    // Configurando os jogadores
    players[0].id = 1;
    players[0].address.sin_family = AF_INET;
    players[0].address.sin_addr.s_addr = inet_addr("127.0.0.1");
    players[0].port = 8080;
    players[0].address.sin_port = htons(players[0].port);

    players[1].id = 2;
    players[1].address.sin_family = AF_INET;
    players[1].address.sin_addr.s_addr = inet_addr("127.0.0.1"); // Use the same IP but different port
    players[1].port = 8081;
    players[1].address.sin_port = htons(players[1].port);

    Network net1, net2;

    printf("Initializing network for player 1...\n");
    init_network(&net1, players, 2);
    init_network_process(&net1);
    printf("Network for player 1 initialized.\n");

    printf("Initializing network for player 2...\n");
    init_network(&net2, players, 2);
    init_network_process(&net2);
    printf("Network for player 2 initialized.\n");

    return 0;
}
