#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <stdint.h>

#define BUF_SIZE 1024
#define PORT 5000

typedef struct {
    uint8_t start_marker;       // 1 byte
    uint32_t origin;            // 4 bytes
    uint32_t destination;       // 4 bytes
    uint8_t card_action;        // 1 byte (4 bits para card e 4 bits para type)
    uint8_t receive_confirmation; // 1 bit (usaremos 1 byte para simplificação)
    uint8_t end_marker;         // 1 byte
} packet_t;

typedef struct {
    char ip[16];
    int port;
    int has_token;
} Node;

Node nodes[4];
int current_node_index = 0;

int sock;
struct sockaddr_in addr;

void read_nodes_from_file(const char *filename) {
    FILE *file = fopen(filename, "r");
    if (!file) {
        perror("fopen");
        exit(1);
    }

    for (int i = 0; i < 4; i++) {
        if (fscanf(file, "%15s %d", nodes[i].ip, &nodes[i].port) != 2) {
            fprintf(stderr, "Error reading node %d from file\n", i);
            fclose(file);
            exit(1);
        }
    }

    fclose(file);
}

void init_socket() {
    sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock < 0) {
        perror("socket");
        exit(1);
    }

    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    addr.sin_port = htons(nodes[current_node_index].port);

    if (bind(sock, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
        perror("bind");
        close(sock);
        exit(1);
    }

    printf("Socket initialized on IP: %s, Port: %d\n", nodes[current_node_index].ip, nodes[current_node_index].port);
}

packet_t create_packet(uint32_t origin, uint32_t destination, uint8_t card, uint8_t type, uint8_t receive_confirmation) {
    packet_t packet;
    packet.start_marker = 0x7E; // Exemplo de start marker
    packet.origin = htonl(origin);
    packet.destination = htonl(destination);
    packet.card_action = (card << 4) | (type & 0x0F); // Combina card e type em um único byte
    packet.receive_confirmation = receive_confirmation;
    packet.end_marker = 0x7E; // Exemplo de end marker
    return packet;
}

void send_packet(int sock, const packet_t *packet, const struct sockaddr_in *dest_addr) {
    int bytes_sent = sendto(sock, packet, sizeof(packet_t), 0, (struct sockaddr *)dest_addr, sizeof(*dest_addr));
    if (bytes_sent < 0) {
        perror("sendto");
    } else {
        printf("Packet sent to %s:%d\n", inet_ntoa(dest_addr->sin_addr), ntohs(dest_addr->sin_port));
    }
}

int receive_packet(int sock, packet_t *packet) {
    struct sockaddr_in sender_addr;
    socklen_t sender_len = sizeof(sender_addr);
    int recv_len = recvfrom(sock, packet, sizeof(packet_t), 0, (struct sockaddr *)&sender_addr, &sender_len);
    if (recv_len > 0) {
        packet->origin = ntohl(packet->origin);
        packet->destination = ntohl(packet->destination);
        printf("Packet received from %s:%d\n", inet_ntoa(sender_addr.sin_addr), ntohs(sender_addr.sin_port));
    } else {
        perror("recvfrom");
    }
    return recv_len;
}

Node get_next_node() {
    return nodes[(current_node_index + 1) % 4];
}

void send_token() {
    Node next_node = get_next_node();
    packet_t packet = create_packet(current_node_index, (current_node_index + 1) % 4, 0, 0, 0);
    struct sockaddr_in next_addr;
    memset(&next_addr, 0, sizeof(next_addr));
    next_addr.sin_family = AF_INET;
    next_addr.sin_port = htons(next_node.port);
    inet_pton(AF_INET, next_node.ip, &next_addr.sin_addr);

    send_packet(sock, &packet, &next_addr);
}

void start_game() {
    if (current_node_index == 0) {
        printf("Node %d is starting the game.\n", current_node_index);
        nodes[current_node_index].has_token = 1;
        if(nodes[current_node_index].has_token == 1) {
            printf("Node %d has the token.\n", current_node_index);
            send_token();
    }
    }
}
void handle_received_packet(packet_t *packet) {
    printf("Received packet from %d to %d\n", packet->origin, packet->destination);
    // Processar a mensagem e reenviar, se necessário
    if (packet->destination == current_node_index) {
        // Processar a mensagem
        printf("Processing packet at node %d\n", current_node_index);
    } else {
        // Reenviar para o próximo nó
        send_token();
    }
}

int main(int argc, char *argv[]) {
    if (argc != 3) {
        fprintf(stderr, "Usage: %s <node_index> <config_file>\n", argv[0]);
        exit(1);
    }

    current_node_index = atoi(argv[1]);
    if (current_node_index < 0 || current_node_index >= 4) {
        fprintf(stderr, "Invalid node index. Must be 0-3.\n");
        exit(1);
    }

    read_nodes_from_file(argv[2]);

    init_socket();
    start_game();

    while (1) {
        packet_t packet;
        int recv_len = receive_packet(sock, &packet);
        if (recv_len > 0) {
            handle_received_packet(&packet);
        }
    }

    close(sock);
    return 0;
}
