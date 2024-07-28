#ifndef NETWORK_H
#define NETWORK_H

#include <arpa/inet.h>
#include "packet.h"
#include "action.h"
#include "rank.h"

typedef struct {
    struct sockaddr_in address;
    int id;
    uint16_t port;
} Player;

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

void init_network(Network *net, Player *players, int num_players);
void init_network_process(Network *net);
void get_current_node_info(Network *net);
int get_my_id(Network *net);
uint16_t get_my_port(Network *net);
void get_next_node_info(Network *net);
Packet receive_packet(Network *net);
void set_token(Network *net, int token);
int im_the_first_node(Network *net);
Packet receive_packet_and_pass_forward(Network *net);
void timeout_error();
void unexpected_packet_error(int expected_packet, int received_packet);
void send_packet(Network *net, int action, int destination, int received_confirmation);
void send_packet_and_wait_for_response(Network *net, int origin, int destination, int action, int card, int quantity, int num_jesters, int received_confirmation);
struct sockaddr_in id_to_address(Network *net, int id);
int has_token(Network *net);

#endif // NETWORK_H
