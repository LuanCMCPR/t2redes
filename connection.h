#ifndef CONNECTION_H
#define CONNECTION_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <unistd.h>
#include <errno.h>
#include <stdbool.h>

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
#define NUM_ROUNDS 13

/* Actions */
#define SEND_CARD 0x00
#define SEND_END 0x01
#define PREDCTION 0x03
#define SEND_TOKEN 0x04

#define MAKE_PREDICTION 0x02
#define SHOW_PREDICTION 0x05
#define PLAY_CARD 0x06
#define END_ROUND 0x07
#define END_MATCH 0x08

#define CARD_SUIT_MASK  0x03  // Mask for the 2-bit suit
#define CARD_VALUE_MASK 0xFC  // Mask for the 6-bit value (invert to clear bits 0-1)

/* Game phase */
#define DISTRIBUTE_CARDS 100
#define MAKE_PREDICTION_PHASE 101
#define PLAY_CARD_PHASE 102
#define END_ROUND_PHASE 103



typedef struct
{
    int value;
    int suit;
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
    uint8_t origin; // 1 bytes
    uint8_t destination; // 1 bytes
    uint8_t card; // 2 bits for suit and 4 bits for value
    uint8_t data[4];
    uint8_t type; // 4 bits
    uint8_t receive_confirmation; // 1 bit;
    uint8_t end_marker; // 1 byte
} packet_t;

typedef struct
{
    struct sockaddr_in next_node_addr;
    struct sockaddr_in current_node_addr;
    int socket_fd;
    int node_id;
    int num_nodes;
    node_t *players;
    int card_dealer;
    int round;
    int last_winner;
    int score[MAX_PLAYERS];
    uint8_t predictions[MAX_PLAYERS];
    int lifes[MAX_PLAYERS];
    int token;
    packet_t *packet;
    deck_t *deck;
} network_t;


/* Functions for network */
int create_socket();
network_t *network_config(node_t *players, int num_players, int index);
void print_node(node_t *node);
void print_network(network_t *net);
void load_config(const char* filename, node_t *players, int num_players);
void init_network(network_t *net);
packet_t *init_packet();
packet_t *create_or_modify_packet(packet_t *p, int origin, int destination, uint8_t *data, int type);
int has_token(network_t *net);
int send_packet(network_t *net, packet_t *packet);
int send_packet_and_wait(network_t *net, packet_t *response, packet_t *packet);
int receive_packet(network_t *net, packet_t *packet);
int receive_packet_and_pass_forward(network_t *net);
void pass_token(network_t *net);
/* ------------------------------*/

/* Functions for game */
deck_t *create_deck();
void init_deck_player(network_t *net);
int shuffle_deck(deck_t *deck);
void distribute_cards(network_t *net, deck_t *deck);
void print_card(card_t card);
void set_card(uint8_t *card, int suit, int value);
void retrieve_card(uint8_t card,int *suit, int *value);
void match_end(network_t *net);
void end_round(network_t *net);
void show_round_results(network_t *net);
int calculate_results(network_t *net);
card_t get_card(deck_t *deck);
void show_played_card(network_t *net);
int calculate_prediction(network_t *net);
void predictions(network_t *net);
void play_round(network_t *net);
/*------------------------------*/


#endif
