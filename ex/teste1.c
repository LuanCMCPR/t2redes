#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include "packet.h"


typedef struct {
    struct sockaddr_in current_node_addr;
    struct sockaddr_in next_node_addr;
    int socket_fd;
    int node_id;
    Packet packet;
    int token;
    Player *players;
    int num_players;
} Network;

void init_network(Network *net, Player *players, int num_players) {
    net->players = players;
    net->num_players = num_players;

    // Create socket
    if ((net->socket_fd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        perror("Failed to create socket");
        exit(EXIT_FAILURE);
    }

    // Get current and next node information
    get_current_node_info(net);
    get_next_node_info(net);

    // Validate node information
    if (net->current_node_addr.sin_addr.s_addr == INADDR_NONE || net->current_node_addr.sin_port == 0) {
        fprintf(stderr, "Failed to get the information about this device!\n"
                        "Did you create this player in config.yaml?\nExiting...\n");
        exit(EXIT_FAILURE);
    }

    if (net->next_node_addr.sin_addr.s_addr == INADDR_NONE || net->next_node_addr.sin_port == 0) {
        fprintf(stderr, "Failed to get the information about the next node!\nExiting...\n");
        exit(EXIT_FAILURE);
    }

    if (net->current_node_addr.sin_addr.s_addr == net->next_node_addr.sin_addr.s_addr &&
        net->current_node_addr.sin_port == net->next_node_addr.sin_port) {
        fprintf(stderr, "I'm the only node in the network!\nExiting...\n");
        exit(EXIT_FAILURE);
    }

    // Bind socket
    printf("Binding socket to %s:%d...\n", inet_ntoa(net->current_node_addr.sin_addr), ntohs(net->current_node_addr.sin_port));
    if (bind(net->socket_fd, (const struct sockaddr *) &net->current_node_addr, sizeof(net->current_node_addr)) < 0) {
        perror("Failed to bind socket");
        exit(EXIT_FAILURE);
    }
    printf("Socket bound to %s:%d\n", inet_ntoa(net->current_node_addr.sin_addr), ntohs(net->current_node_addr.sin_port));

    net->node_id = get_my_id(net);
    init_packet(&net->packet, net->current_node_addr.sin_addr.s_addr, 0, ACTION_INIT_NETWORK, RANK_JESTER, 0, 0);
    net->token = im_the_first_node(net);
}

void init_network_process(Network *net) {
    if (has_token(net)) {
        printf("Sending init_network packet...\n");

        while (1) {
            send_packet(net, ACTION_INIT_NETWORK, 0, 0);
            Packet response = receive_packet(net);

            if (response.valid) {
                printf("Received response. Everybody is on the network!\n");
                int check_result = check_all_received(&response, net->num_players, net->node_id);
                if (check_result == -1) {
                    break;
                } else {
                    printf("The player %d didn't mark the packet as received!\n", check_result);
                }
            } else {
                printf("Got no response from the network!. Retrying...\n");
            }
        }

    } else {
        printf("Waiting for init_packet...\n");

        while (1) {
            net->packet = receive_packet_and_pass_forward(net);

            if (net->packet.valid) {
                printf("Init packet received!\n");
                break;
            } else {
                printf("Timeout occurred. Retrying...\n");
            }
        }

        printf("Waiting for the setup to start...\n");
        while (1) {
            net->packet = receive_packet(net);

            if (!net->packet.valid) {
                continue;
            }

            if (net->packet.action == ACTION_START_SETUP) {
                printf("All right! Setup starting...\n");
                mark_packet_as_received(&net->packet, net->node_id);
                send_packet(net, -1, -1, -1);
                break;
            }

            if (net->packet.action == ACTION_INIT_NETWORK) {
                printf("Received init packet again. Passing forward...\n");
                mark_packet_as_received(&net->packet, net->node_id);
                send_packet(net, -1, -1, -1);
            }
        }
    }
}

void get_current_node_info(Network *net) {
    int temp_socket_fd;
    struct sockaddr_in temp_addr;
    socklen_t addr_len = sizeof(temp_addr);

    if ((temp_socket_fd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        perror("Failed to create temporary socket");
        exit(EXIT_FAILURE);
    }

    temp_addr.sin_family = AF_INET;
    temp_addr.sin_addr.s_addr = inet_addr("8.8.8.8");
    temp_addr.sin_port = htons(53);

    if (connect(temp_socket_fd, (const struct sockaddr *) &temp_addr, sizeof(temp_addr)) < 0) {
        perror("Failed to connect to temporary address");
        close(temp_socket_fd);
        exit(EXIT_FAILURE);
    }

    if (getsockname(temp_socket_fd, (struct sockaddr *) &temp_addr, &addr_len) < 0) {
        perror("Failed to get socket name");
        close(temp_socket_fd);
        exit(EXIT_FAILURE);
    }

    net->current_node_addr = temp_addr;
    close(temp_socket_fd);
    net->current_node_addr.sin_port = get_my_port(net);
}

int get_my_id(Network *net) {
    for (int i = 0; i < net->num_players; i++) {
        if (net->players[i].address.s_addr == net->current_node_addr.sin_addr.s_addr) {
            return net->players[i].id;
        }
    }
    fprintf(stderr, "Couldn't find my id!\n");
    exit(EXIT_FAILURE);
}

uint16_t get_my_port(Network *net) {
    for (int i = 0; i < net->num_players; i++) {
        if (net->players[i].address.s_addr == net->current_node_addr.sin_addr.s_addr) {
            return net->players[i].port;
        }
    }
    fprintf(stderr, "Couldn't find my port!\n");
    exit(EXIT_FAILURE);
}

void get_next_node_info(Network *net) {
    for (int i = 0; i < net->num_players; i++) {
        if (net->players[i].address.s_addr == net->current_node_addr.sin_addr.s_addr) {
            net->next_node_addr = net->players[(i + 1) % net->num_players].address;
            net->next_node_addr.sin_port = net->players[(i + 1) % net->num_players].port;
            return;
        }
    }
    fprintf(stderr, "Couldn't find the next node info!\n");
    exit(EXIT_FAILURE);
}

Packet receive_packet(Network *net) {
    Packet packet;
    memset(&packet, 0, sizeof(Packet));
    struct sockaddr_in src_addr;
    socklen_t addr_len = sizeof(src_addr);
    char buffer[100];

    if (recvfrom(net->socket_fd, buffer, sizeof(buffer), 0, (struct sockaddr *) &src_addr, &addr_len) < 0) {
        return packet; // Invalid packet
    }

    convert_bytes_to_packet(buffer, &packet);
    if (!check_packet_validity(&packet)) {
        printf("Invalid packet received!\n");
        memset(&packet, 0, sizeof(Packet));
    }

    return packet;
}

void set_token(Network *net, int token) {
    net->token = token;
}

int im_the_first_node(Network *net) {
    return net->players[0].address.s_addr == net->current_node_addr.sin_addr.s_addr;
}

Packet receive_packet_and_pass_forward(Network *net) {
    net->packet = receive_packet(net);

    if (!net->packet.valid) {
        return net->packet;
    }

    if (net->packet.action == ACTION_PASS_TOKEN && is_for_me(&net->packet, net->current_node_addr.sin_addr.s_addr)) {
        return net->packet;
    }

    mark_packet_as_received(&net->packet, net->node_id);
    send_packet(net, -1, -1, -1);

    return net->packet;
}

void timeout_error() {
    fprintf(stderr, "Timeout!\n");
    exit(EXIT_FAILURE);
}

void unexpected_packet_error(int expected_packet, int received_packet) {
    fprintf(stderr, "Unexpected packet received! Expected: %d, Received: %d\n", expected_packet, received_packet);
    exit(EXIT_FAILURE);
}

void send_packet(Network *net, int action, int destination, int received_confirmation) {
    update_packet(&net->packet, net->current_node_addr.sin_addr.s_addr, destination, action, -1, -1, received_confirmation);

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

    while (1) {
        Packet response = receive_packet(net);
        if (!response.valid) {
            printf("Network timeout!\n");
            exit(EXIT_FAILURE);
        }

        int id_node_didnt_receive = check_all_received(&response, net->num_players, net->node_id);
        if (id_node_didnt_receive != -1) {
            struct sockaddr_in addr_player_didnt_receive = id_to_address(net, id_node_didnt_receive);
            update_packet(&net->packet, origin, addr_player_didnt_receive.sin_addr.s_addr, action, card, quantity, received_confirmation);
            convert_packet_to_bytes(&net->packet, buffer);
            if (sendto(net->socket_fd, buffer, sizeof(buffer), 0, (const struct sockaddr *) &net->next_node_addr, sizeof(net->next_node_addr)) < 0) {
                perror("Failed to send packet");
                exit(EXIT_FAILURE);
            }
        } else {
            break;
        }
    }
}

struct sockaddr_in id_to_address(Network *net, int id) {
    for (int i = 0; i < net->num_players; i++) {
        if (net->players[i].id == id) {
            return net->players[i].address;
        }
    }
    struct sockaddr_in addr;
    addr.sin_addr.s_addr = INADDR_NONE;
    addr.sin_port = 0;
    return addr;
}

int has_token(Network *net) {
    return net->token;
}
