#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <unistd.h>
#include <errno.h>

#include "type.h"
#include "packet.h"
#include "rank.h"

#define BUFFER_SIZE 100

typedef struct {
    char* address;
    int port;
    int id;
} Player;

typedef struct {
    int socket;
    struct sockaddr_in current_node_addr;
    struct sockaddr_in next_node_addr;
    Player* players;
    int num_players;
    int node_id;
    Packet packet;
    int token;
} Network;

void error_exit(const char* msg) {
    perror(msg);
    exit(EXIT_FAILURE);
}

Network* init_network(Player* players, int num_players) {
    Network* network = (Network*)malloc(sizeof(Network));
    if (!network) {
        error_exit("Failed to allocate memory for Network");
    }

    network->players = players;
    network->num_players = num_players;
    
    // Create socket
    network->socket = socket(AF_INET, SOCK_DGRAM, 0);
    if (network->socket < 0) {
        error_exit("Failed to create socket");
    }

    // Get current and next node information
    network->current_node_addr.sin_family = AF_INET;
    network->next_node_addr.sin_family = AF_INET;

    char* current_node_address = get_current_node_info(players, num_players, &(network->current_node_addr.sin_port));
    char* next_node_address = get_next_node_info(players, num_players, &(network->next_node_addr.sin_port));

    if (!current_node_address || !next_node_address) {
        error_exit("Failed to get node information");
    }

    if (strcmp(current_node_address, next_node_address) == 0) {
        error_exit("I'm the only node in the network");
    }

    // Bind socket
    network->current_node_addr.sin_addr.s_addr = inet_addr(current_node_address);
    if (bind(network->socket, (struct sockaddr*)&(network->current_node_addr), sizeof(network->current_node_addr)) < 0) {
        error_exit("Failed to bind socket");
    }

    network->next_node_addr.sin_addr.s_addr = inet_addr(next_node_address);

    network->node_id = get_my_id(players, num_players, current_node_address);
    network->packet = init_packet(current_node_address, 0, INIT_NETWORK, JESTER, 0, 0);
    network->token = im_the_first_node(players, num_players, current_node_address);

    free(current_node_address);
    free(next_node_address);

    return network;
}

void init_network(Network* network) {
    if (network->token) {
        printf("Sending init_network packet...\n");

        while (1) {
            send_packet(network, NULL, INIT_NETWORK, 0, 0, 0, 0);
            Packet* response = receive_packet(network);

            if (response) {
                printf("Received response. Everybody is on the network!\n");
                int check_result = check_all_received(response, network->num_players, network->node_id);
                if (check_result == -1) {
                    free(response);
                    break;
                } else {
                    printf("The player %d didn't mark the packet as received!\n", check_result);
                }
                free(response);
            } else {
                printf("Got no response from the network! Retrying...\n");
            }
        }
    } else {
        printf("Waiting for init_packet...\n");

        while (1) {
            network->packet = receive_packet_and_pass_forward(network);
            if (network->packet) {
                printf("Init packet received!\n");
                break;
            } else {
                printf("Timeout occurred. Retrying...\n");
            }
        }

        printf("Waiting for the setup to start...\n");
        while (1) {
            network->packet = receive_packet(network);
            if (!network->packet) {
                continue;
            }

            if (network->packet->type == START_SETUP) {
                printf("All right! Setup starting...\n");
                mark_packet_as_received(&(network->packet), network->node_id);
                send_packet(network, NULL, 0, 0, 0, 0, 0);
                break;
            }

            if (network->packet->type == INIT_NETWORK) {
                printf("Received init packet again. Passing forward...\n");
                mark_packet_as_received(&(network->packet), network->node_id);
                send_packet(network, NULL, 0, 0, 0, 0, 0);
            }
        }
    }
}

char* get_current_node_info(Player* players, int num_players, int* port) {
    int temp_socket = socket(AF_INET, SOCK_DGRAM, 0);
    if (temp_socket < 0) {
        error_exit("Failed to create temporary socket");
    }

    struct sockaddr_in temp_addr;
    temp_addr.sin_family = AF_INET;
    temp_addr.sin_port = htons(1);
    temp_addr.sin_addr.s_addr = inet_addr("8.8.8.8");

    if (connect(temp_socket, (struct sockaddr*)&temp_addr, sizeof(temp_addr)) < 0) {
        close(temp_socket);
        error_exit("Failed to connect temporary socket");
    }

    struct sockaddr_in addr;
    socklen_t addr_len = sizeof(addr);
    if (getsockname(temp_socket, (struct sockaddr*)&addr, &addr_len) < 0) {
        close(temp_socket);
        error_exit("Failed to get socket name");
    }

    char* address = inet_ntoa(addr.sin_addr);
    printf("This machine IP: %s\n", address);
    *port = get_my_port(players, num_players, address);

    close(temp_socket);
    return strdup(address);
}

int get_my_id(Player* players, int num_players, char* address) {
    for (int i = 0; i < num_players; i++) {
        if (strcmp(players[i].address, address) == 0) {
            return players[i].id;
        }
    }
    error_exit("Couldn't find my id!");
}

int get_my_port(Player* players, int num_players, char* address) {
    for (int i = 0; i < num_players; i++) {
        if (strcmp(players[i].address, address) == 0) {
            return players[i].port;
        }
    }
    return -1;
}

char* get_next_node_info(Player* players, int num_players, int* port) {
    for (int i = 0; i < num_players; i++) {
        if (strcmp(players[i].address, players[(i + 1) % num_players].address) == 0) {
            *port = players[(i + 1) % num_players].port;
            return strdup(players[(i + 1) % num_players].address);
        }
    }
    return NULL;
}

Packet* receive_packet(Network* network) {
    char buffer[BUFFER_SIZE];
    struct sockaddr_in sender_addr;
    socklen_t addr_len = sizeof(sender_addr);

    int bytes_received = recvfrom(network->socket, buffer, BUFFER_SIZE, 0, (struct sockaddr*)&sender_addr, &addr_len);
    if (bytes_received < 0) {
        if (errno == EWOULDBLOCK) {
            return NULL;
        } else {
            error_exit("Failed to receive packet");
        }
    }

    Packet* packet = convert_bytes_to_packet(buffer);
    if (!check_packet_validity(packet)) {
        printf("Invalid packet received!\n");
        free(packet);
        return NULL;
    }

    return packet;
}

void set_token(Network* network, int token) {
    network->token = token;
}

int im_the_first_node(Player* players, int num_players, char* address) {
    return strcmp(players[0].address, address) == 0;
}

Packet* receive_packet_and_pass_forward(Network* network) {
    network->packet = receive_packet(network);
    if (!network->packet) {
        return NULL;
    }

    if (network->packet->type == PASS_TOKEN && is_for_me(network->packet, network->current_node_addr)) {
        return network->packet;
    }

    mark_packet_as_received(&(network->packet), network->node_id);
    send_packet(network, NULL, 0, 0, 0, 0, 0);

    return network->packet;
}

void timeout_error() {
    error_exit("Timeout!");
}

void unexpected_packet_error(int expected_packet, int received_packet) {
    fprintf(stderr, "Unexpected packet received! Expected: %d, Received: %d\n", expected_packet, received_packet);
    exit(EXIT_FAILURE);
}

void send_packet(Network* network, char* destination, int type, int card, int quantity, int num_jesters, int received_confirmation) {
    update_packet(&(network->packet), destination, type, card, quantity, num_jesters, received_confirmation);
    char* packet_bytes = convert_packet_to_bytes(&(network->packet));
    if (sendto(network->socket, packet_bytes, BUFFER_SIZE, 0, (struct sockaddr*)&(network->next_node_addr), sizeof(network->next_node_addr)) < 0) {
        error_exit("Failed to send packet");
    }
}

void send_packet_and_wait_for_response(Network* network, char* origin, char* destination, int type, int card, int quantity, int num_jesters, int received_confirmation) {
    update_packet(&(network->packet), origin, destination, type, card, quantity, num_jesters, received_confirmation);
    char* packet_bytes = convert_packet_to_bytes(&(network->packet));

    if (sendto(network->socket, packet_bytes, BUFFER_SIZE, 0, (struct sockaddr*)&(network->next_node_addr), sizeof(network->next_node_addr)) < 0) {
        error_exit("Failed to send packet");
    }

    while (1) {
        Packet* response = receive_packet(network);
        if (!response) {
            printf("Network timeout!\n");
            error_exit("Network timeout!");
        }

        int id_node_didnt_receive = check_all_received(response, network->num_players, network->node_id);
        if (id_node_didnt_receive != -1) {
            char* address_player_didnt_receive = id_to_address(network, id_node_didnt_receive);
            update_packet(&(network->packet), address_player_didnt_receive, 0, 0, 0, 0, 0);
            sendto(network->socket, packet_bytes, BUFFER_SIZE, 0, (struct sockaddr*)&(network->next_node_addr), sizeof(network->next_node_addr));
            free(address_player_didnt_receive);
        } else {
            free(response);
            break;
        }
        free(response);
    }
}

char* id_to_address(Network* network, int id) {
    for (int i = 0; i < network->num_players; i++) {
        if (network->players[i].id == id) {
            return strdup(network->players[i].address);
        }
    }
    return NULL;
}

int has_token(Network* network) {
    return network->token;
}
