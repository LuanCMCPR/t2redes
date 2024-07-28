#include "network.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

void init_network(Network *net, Player *players, int num_players) {
    net->players = players;
    net->num_players = num_players;

    net->socket_fd = socket(AF_INET, SOCK_DGRAM, 0);
    if (net->socket_fd < 0) {
        perror("Failed to create socket");
        exit(EXIT_FAILURE);
    }

    get_current_node_info(net);
    get_next_node_info(net);

    if (net->current_node_addr.sin_addr.s_addr == net->next_node_addr.sin_addr.s_addr) {
        printf("I'm the only node in the network! Exiting...\n");
        exit(EXIT_FAILURE);
    }

    if (bind(net->socket_fd, (const struct sockaddr *) &net->current_node_addr, sizeof(net->current_node_addr)) < 0)
    {
        perror("Failed to bind socket");
        exit(EXIT_FAILURE);
    }

    net->node_id = get_my_id(net);
    set_token(net, im_the_first_node(net));
}

void init_network_process(Network *net) {
    if (has_token(net)) {
        printf("Sending init_network packet...\n");

        while (1) {
            send_packet(net, ACTION_INIT_NETWORK, 0, 0);
            Packet response = receive_packet(net);

            if (response.valid) {
                printf("Received response. Everybody is on the network!\n");
                if (check_all_received(&response, net->num_players, net->node_id) == -1) {
                    break;
                } else {
                    printf("A player didn't mark the packet as received!\n");
                }
            }
            printf("Got no response from the network. Retrying...\n");
        }
    } else {
        printf("Waiting for init_packet...\n");

        while (1) {
            Packet packet = receive_packet_and_pass_forward(net);
            if (packet.valid && packet.action == ACTION_INIT_NETWORK) {
                printf("Init packet received!\n");
                break;
            }
            printf("Timeout occurred. Retrying...\n");
        }

        printf("Waiting for the setup to start...\n");
        while (1) {
            Packet packet = receive_packet(net);
            if (packet.valid) {
                if (packet.action == ACTION_START_SETUP) {
                    printf("All right! Setup starting...\n");
                    packet.receive_confirmation = 1;
                    send_packet(net, packet.action, packet.destination, packet.receive_confirmation);
                    break;
                }
                if (packet.action == ACTION_INIT_NETWORK) {
                    printf("Received init packet again. Passing forward...\n");
                    packet.receive_confirmation = 1;
                    send_packet(net, packet.action, packet.destination, packet.receive_confirmation);
                }
            }
        }
    }
}

void get_current_node_info(Network *net) {
    net->current_node_addr.sin_family = AF_INET;
    net->current_node_addr.sin_addr.s_addr = inet_addr("127.0.0.1"); // Example IP address
    net->current_node_addr.sin_port = htons(get_my_port(net));
}

int get_my_id(Network *net) {
    for (int i = 0; i < net->num_players; i++) {
        if (net->players[i].address.sin_addr.s_addr == net->current_node_addr.sin_addr.s_addr) {
            return net->players[i].id;
        }
    }
    printf("Couldn't find my id! Exiting...\n");
    exit(EXIT_FAILURE);
}

uint16_t get_my_port(Network *net) {
    for (int i = 0; i < net->num_players; i++) {
        if (net->players[i].address.sin_addr.s_addr == net->current_node_addr.sin_addr.s_addr) {
            return net->players[i].port;
        }
    }
    printf("Couldn't find my port! Exiting...\n");
    exit(EXIT_FAILURE);
}



void get_next_node_info(Network *net) {
    for (int i = 0; i < net->num_players; i++) {
        if (net->players[i].address.sin_addr.s_addr == net->current_node_addr.sin_addr.s_addr){
            net->next_node_addr = net->players[(i + 1) % net->num_players].address;
            return;
        }
    }
    printf("Couldn't find the next node! Exiting...\n");
    exit(EXIT_FAILURE);
}

Packet receive_packet(Network *net) {
    char buffer[100];
    socklen_t len = sizeof(net->next_node_addr);
    int n = recvfrom(net->socket_fd, buffer, sizeof(buffer), 0, (struct sockaddr *) &net->next_node_addr, &len);
    if (n < 0) {
        Packet invalid_packet;
        invalid_packet.valid = 0;
        return invalid_packet;
    }
    return convert_bytes_to_packet(buffer);
}

void set_token(Network *net, int token) {
    net->token = token;
}

int im_the_first_node(Network *net) {
    return net->players[0].address.sin_addr.s_addr == net->current_node_addr.sin_addr.s_addr;
}

Packet receive_packet_and_pass_forward(Network *net) {
    Packet packet = receive_packet(net);
    if (!packet.valid) {
        return packet;
    }

    if (packet.action == ACTION_PASS_TOKEN && packet.destination == net->current_node_addr.sin_addr.s_addr) {
        return packet;
    }

    update_packet(&packet, net->packet.origin, net->packet.destination, net->packet.action, net->packet.card, net->packet.receive_confirmation, 0);
    char buffer[100];
    convert_packet_to_bytes(&packet, buffer);
    if (sendto(net->socket_fd, buffer, sizeof(buffer), 0, (const struct sockaddr *) &net->next_node_addr, sizeof(net->next_node_addr)) < 0) {
        perror("Failed to send packet");
        exit(EXIT_FAILURE);
    }

    return packet;
}

void send_packet(Network *net, int action, int destination, int received_confirmation) {
    update_packet(&net->packet, net->current_node_addr.sin_addr.s_addr, destination, action, 0, 0, received_confirmation);
    char buffer[100];
    convert_packet_to_bytes(&net->packet, buffer);
    if (sendto(net->socket_fd, buffer, sizeof(buffer), 0, (const struct sockaddr *) &net->next_node_addr, sizeof(net->next_node_addr)) < 0) {
        perror("Failed to send packet");
        exit(EXIT_FAILURE);
    }
}

void send_packet_and_wait_for_response(Network *net, int origin, int destination, int action, int card, int quantity, int num_jesters, int received_confirmation) {
    update_packet(&net->packet, origin, destination, action, card, quantity, received_confirmation);
    char buffer[100];
    convert_packet_to_bytes(&net->packet, buffer);
    if (sendto(net->socket_fd, buffer, sizeof(buffer), 0, (const struct sockaddr *) &net->next_node_addr, sizeof(net->next_node_addr)) < 0) {
        perror("Failed to send packet");
        exit(EXIT_FAILURE);
    }

    Packet response = receive_packet(net);
    if (!response.valid) {
        timeout_error();
    }

    if (response.action != action || response.origin != destination) {
        unexpected_packet_error(action, response.action);
    }
}

struct sockaddr_in id_to_address(Network *net, int id) {
    for (int i = 0; i < net->num_players; i++) {
        if (net->players[i].id == id) {
            return net->players[i].address;
        }
    }
    printf("Couldn't find the address for id %d! Exiting...\n", id);
    exit(EXIT_FAILURE);
}

int has_token(Network *net) {
    return net->token;
}

void timeout_error() {
    printf("Timeout occurred while waiting for a packet.\n");
    exit(EXIT_FAILURE);
}

void unexpected_packet_error(int expected_packet, int received_packet) {
    printf("Unexpected packet received. Expected: %d, Received: %d\n", expected_packet, received_packet);
    exit(EXIT_FAILURE);
}
