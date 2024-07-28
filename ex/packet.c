#include "packet.h"
#include <string.h>

void update_packet(Packet *packet, uint32_t origin, uint32_t destination, uint8_t action, uint8_t card, uint8_t quantity, uint8_t receive_confirmation) {
    packet->origin = origin;
    packet->destination = destination;
    packet->action = action;
    packet->card = card;
    packet->receive_confirmation = receive_confirmation;
}

void convert_packet_to_bytes(Packet *packet, char *buffer) {
    memcpy(buffer, packet, sizeof(Packet));
}

Packet convert_bytes_to_packet(char *buffer) {
    Packet packet;
    memcpy(&packet, buffer, sizeof(Packet));
    packet.valid = 1; // Simplistic validity check
    return packet;
}

int check_packet_validity(Packet *packet) {
    return packet->valid;
}

int check_all_received(Packet *packet, int num_players, int node_id) {
    // Simplistic implementation, should be expanded based on actual logic
    return -1;
}
