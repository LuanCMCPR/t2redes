#ifndef CONNECTION_H
#define CONNECTION_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <unistd.h>
#include <errno.h>

/* For Network */
#define INIT_NETWORK 0
#define START_SETUP 1
#define START_MARKER 0x01
#define END_MARKER 0x02

/* For Game */
#define NUM_CARDS 52
#define SEQ_TOTAL 13
#define NUM_SUITS 4
#define NUM_RANKS 13
#define MAX_PLAYERS 4
#define NUM_PLAYER_CARDS 13
#define SEND_CARD 0x00

typedef struct
{
    int value;
    int suit;
    int rank;
} card_t;

typedef struct
{
    int size;
    card_t *cards;
} deck_t;


typedef struct 
{
    int id;
    struct sockaddr_in current;
    char ip[16];
    int port;
    struct sockaddr_in next;
    char next_ip[16];
    int next_port;

} node_t;


typedef struct
{
    uint8_t start_marker; // 1 byte
    // uint8_t origin[4]; // 16 bytes
    // uint8_t destination[4]; // 16 bytes
    uint8_t origin; // 4 bytes
    uint8_t destination; // 4 bytes
    uint8_t card; // 4 bits
    uint8_t type; // 4 bits
    uint8_t receive_confirmation; // 1 bit;
    uint8_t end_marker; // 1 byte
} packet_t;

typedef struct
{
    struct sockaddr_in next_node_addr;
    struct sockaddr_in current_node_addr;
    packet_t *packet;
    int socket_fd;
    int node_id;
    int num_nodes;
    node_t *players;
    int token;
    deck_t *deck;
} network_t;

// typedef struct {
//     uint8_t start_marker;       // 1 byte
//     uint32_t origin;            // 4 bytes
//     uint32_t destination;       // 4 bytes
//     uint8_t card_action;        // 1 byte (4 bits para card e 4 bits para type)
//     uint8_t receive_confirmation; // 1 bit (usaremos 1 byte para simplificação)
//     uint8_t end_marker;         // 1 byte
// } packet_t;

/* Functions for network */
int create_socket();
network_t *network_config(node_t *players, int num_players, int index);
void print_node(node_t *node);
void print_network(network_t *net);
void print_packet(packet_t *p);
void print_packet2(const packet_t *p);
void load_config(const char* filename, node_t *players, int num_players);
void init_network(network_t *net);
packet_t *init_packet();
// packet_t *create_or_modify_packet(packet_t *p, char *origin_addr, char *destination_addr, int card, int type);
packet_t *create_or_modify_packet(packet_t *p, int origin, int destination, int card, int type);
int check_packet(packet_t *p);
int send_packet(network_t *net, packet_t *packet);
int send_packet_and_wait(network_t *net, packet_t *response, packet_t *packet);
int receive_packet(network_t *net, packet_t *packet);
int receive_packet_and_pass_forward(network_t *net);
/* ------------------------------*/

/* Functions for game */
deck_t *create_deck();
void init_deck_player(network_t *net);
int shuffle_deck(deck_t *deck);
void distribute_cards(network_t *net, deck_t *deck);
void print_deck(deck_t *deck);
/*------------------------------*/

char *get_current_node_info(node_t *players, int num_players, int *port);
int get_my_port(node_t *players, int num_players, char* address );
char *get_next_node_info(node_t *players, int num_players, int *port);
int has_token(network_t *net);
int is_first_node(node_t *players, int num_players);
int get_my_id(node_t *players, int num_players, char *address);
int receive_packet_and_pass_forward(network_t *net);
int mark_packet_as_received(packet_t *p);
int is_all_ready(packet_t *response, int num_nodes, int node_id);
#endif
