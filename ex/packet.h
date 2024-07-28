#ifndef PACKET_H
#define PACKET_H

#include <stdint.h>
#include <arpa/inet.h>

typedef struct {
    uint8_t start_marker;
    uint32_t origin;
    uint32_t destination;
    uint8_t card;
    uint8_t action;
    uint8_t receive_confirmation;
    uint8_t end_marker;
    int valid;
} Packet;

void update_packet(Packet *packet, uint32_t origin, uint32_t destination, uint8_t action, uint8_t card, uint8_t quantity, uint8_t receive_confirmation);
void convert_packet_to_bytes(Packet *packet, char *buffer);
Packet convert_bytes_to_packet(char *buffer);
int check_packet_validity(Packet *packet);
int check_all_received(Packet *packet, int num_players, int node_id);

#endif // PACKET_H
