#include "connection.h"


// network_t *network_config(const char *filename)
// {
    

//     net->socket_fd = create_socket();
//     net->current_node_index = 0;
//     net->node_id = net->nodes[0].id;
//     net->token = 1;

//     memset(&net->current_node_addr, 0, sizeof(net->current_node_addr));
//     net->current_node_addr.sin_family = AF_INET;
//     net->current_node_addr.sin_addr.s_addr = htonl(INADDR_ANY);
//     net->current_node_addr.sin_port = htons(net->nodes[0].port);

//     if (bind(net->socket_fd, (struct sockaddr *)&net->current_node_addr, sizeof(net->current_node_addr)) < 0) {
//         perror("Failed to bind socket");
//         exit(EXIT_FAILURE);
//     }

//     printf("Socket initialized on IP: %s, Port: %d\n", net->nodes[0].ip, net->nodes[0].port);

//     return net;
// }



/* Create a UDP DATAGRAM socket */
int create_socket()
{
    int sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock < 0)
    {
        perror("Failed to create socket");
        exit(EXIT_FAILURE);
    }

    return sock;
}

/* Configure node network */
network_t *network_config(node_t *players, int num_players, int index)
{
    network_t *net = (network_t *)malloc(sizeof(network_t));
    if (!net)
    {
        perror("Failed to allocate memory for network");
        exit(EXIT_FAILURE);
    }

    net->socket_fd = create_socket();
    net->num_nodes = num_players;
    net->players = players;
    net->node_id = players[index].id;

    memset(&net->current_node_addr, 0, sizeof(net->current_node_addr));
    memset(&net->next_node_addr, 0, sizeof(net->next_node_addr));
    
    /* Setting address family */
    net->current_node_addr.sin_family = AF_INET;
    net->current_node_addr.sin_addr.s_addr = inet_addr(players[index].ip);
    net->current_node_addr.sin_port = htons(players[index].port);

    if(bind(net->socket_fd, (struct sockaddr *)&net->current_node_addr, sizeof(net->current_node_addr)) < 0) {
        perror("Failed to bind socket");
        return NULL;
    }

    net->next_node_addr.sin_family = AF_INET;
    net->next_node_addr.sin_addr.s_addr = inet_addr(players[index].next_ip);
    net->next_node_addr.sin_port = htons(players[index].next_port);

    // net->next_node_addr.sin_family = AF_INET;
    // /* Getting addrs */
    // int port;
    // char* current_node_address = get_current_node_info(players, num_players, &port );
    // char* next_node_address = get_next_node_info(players, num_players, &port );
    // net->next_node_addr.sin_port = htons(port);

    /* Check if the node is the only one in the network */
    // if (!current_node_address || !next_node_address)
    // {
    //     perror("Failed to get node information");
    //     return NULL;
    // }

    // if (strcmp(current_node_address, next_node_address) == 0)
    // {
    //     perror("I'm the only node in the network");
    //     return NULL;
    // }

    if (strcmp(players[index].ip, players[index].next_ip) == 0)
        if(players[index].port == players[index].next_port)
        {
            perror("I'm the only node in the network");
            return NULL;
        }

    // /* Setting the current node address */
    // net->current_node_addr.sin_addr.s_addr = inet_addr(current_node_address);
    // if (bind(net->socket_fd, (struct sockaddr *)&net->current_node_addr, sizeof(net->current_node_addr)) < 0) {
    //     perror("Failed to bind socket");
    //     return NULL;
    // }

    /* Setting the next node address */
    // net->next_node_addr.sin_addr.s_addr = inet_addr(next_node_address);

    /* Setting the node id and token */
    // net->node_id = get_my_id(players, num_players, current_node_address);
    // net->token = is_first_node(players, num_players);
    // net->packet = create_packet(net->node_id, net->node_id, 0, 0, 0);

    // free(current_node_address);
    // free(next_node_address);

    // Player 0 always initializes with the token
    if(players[index].id == 1)
        net->token = 1;
    else
        net->token = 0;
    
    net->packet = init_packet();

    init_deck_player(net);

    return net;
} 


void print_node(node_t *node)
{
    printf("Node ID: %d\n", node->id);
    printf("Node IP: %s\n", node->ip);
    printf("Node Port: %d\n", node->port);
    printf("Next Node IP: %s\n", node->next_ip);
    printf("Next Node Port: %d\n", node->next_port);
}

void print_network(network_t *net)
{
    printf("Node ID: %d\n", net->node_id);
    printf("Token: %d\n", net->token);
    printf("Number of nodes: %d\n", net->num_nodes);
    printf("Current node IP: %s\n", inet_ntoa(net->current_node_addr.sin_addr));
    printf("Current node Port: %d\n", ntohs(net->current_node_addr.sin_port));
    printf("Next node IP: %s\n", inet_ntoa(net->next_node_addr.sin_addr));
    printf("Next node Port: %d\n", ntohs(net->next_node_addr.sin_port));
}

// void print_packet(packet_t *p)
// {
//     printf("Packet\n");
//     printf("Origin: %s\n", p->origin);
//     printf("Destination: %s\n", p->destination);
//     printf("Card: %d\n", p->card);
//     printf("Type: %d\n", p->type);
// }

// void print_packet2(const packet_t *p) {
//     char origin_str[INET_ADDRSTRLEN];
//     char destination_str[INET_ADDRSTRLEN];
    
//     // Convert origin IP to string
//     struct in_addr origin_addr;
//     memcpy(&origin_addr, p->origin, sizeof(origin_addr));
//     if (inet_ntop(AF_INET, &origin_addr, origin_str, sizeof(origin_str)) == NULL) {
//         perror("inet_ntop");
//         return;
//     }
    
//     // Convert destination IP to string
//     struct in_addr destination_addr;
//     memcpy(&destination_addr, p->destination, sizeof(destination_addr));
//     if (inet_ntop(AF_INET, &destination_addr, destination_str, sizeof(destination_str)) == NULL) {
//         perror("inet_ntop");
//         return;
//     }
    
//     // Print packet information
//     printf("Packet Information:\n");
//     printf("  Origin IP:        %s\n", origin_str);
//     printf("  Destination IP:   %s\n", destination_str);
//     printf("  Card:             %u\n", p->card);
//     printf("  Type:             %u\n", p->type);
//     printf("  Receive Confirmation: %u\n", p->receive_confirmation);
//     printf("  End Marker:       %u\n", p->end_marker);
// }

void print_packet(packet_t *p) {
    
    // Print packet information
    printf("Packet Information:\n");
    printf("  Origin IP:        %d\n", p->origin);
    printf("  Destination IP:   %d\n", p->destination);
    printf("  Card:             %d\n", p->card);
    printf("  Type:             %d\n", p->type);
    printf("  Receive Confirmation: %d\n", p->receive_confirmation);
    printf("  End Marker:       %u\n", p->end_marker);
}



void load_config(const char* filename, node_t *players, int num_players)
{
    FILE *file = fopen(filename, "r");
    if (file == NULL) {
        perror("Unable to open config file");
        exit(EXIT_FAILURE);
    }

    int count = 0;
    
    while (fscanf(file, "%d %s %d %s %d", &players[count].id, players[count].ip, &players[count].port, players[count].next_ip, &players[count].next_port) != EOF) {
        if (players->id == num_players) {
            fclose(file);
            return;
        }
        count++;
    }
}

void init_network(network_t *net)
{
    if(has_token(net))
    {
        printf("I node %d have the token\n", net->node_id);
        printf("Sending start packet\n");

        while(1)
        {
            // net->packet = create_or_modify_packet(NULL, net->players[net->node_id-1].ip, net->players[net->node_id-1].next_ip, 0, INIT_NETWORK);
            net->packet = create_or_modify_packet(NULL, net->players[net->node_id-1].id, net->players[net->node_id].id , 0, INIT_NETWORK);
            packet_t *response = init_packet();
            send_packet_and_wait(net, response, net->packet);
    
            if(response)
            {
                printf("Received response.Everyone is ready\n");
                free(response); 
                return;
            } 
            else
            {
                printf("No response received\n");
            }
        }
    }
    else
    {
        printf("I don't have the token, waiting for start packet\n");
        net->packet = create_or_modify_packet(NULL, net->players[net->node_id-1].id, net->players[net->node_id].id, 0,0);
        receive_packet(net, net->packet);
        send_packet(net, net->packet);
        // if(net->packet->type == INIT_NETWORK)
        // {
            // printf("Received init packet\n");
            // mark_packet_as_received(net->packet);
            // send_packet(net, NULL);
        // }
        // while(1)
        // {
            // receive_packet_and_pass_forward(net);
            // if (net->packet->type == INIT_NETWORK)
            // {
                // printf("Init packet received!\n");
                // break;
            // }
        // }
        return;

        printf("Waiting for the setup to start...\n");
        while(1)
        {
            receive_packet(net, net->packet);
            if (!net->packet)
                continue;

            if (net->packet->type == START_SETUP)
            {
                printf("All right! Setup starting...\n");
                mark_packet_as_received(net->packet);
                send_packet(net, NULL);
                break;
            }

            if (net->packet->type == INIT_NETWORK)
            {
                printf("Received init packet again. Passing forward...\n");
                mark_packet_as_received(net->packet);
                send_packet(net, NULL);
            }
        }
    }
}

packet_t *init_packet()
{
    packet_t * packet;

    if((packet = malloc(sizeof(packet_t))) == NULL)
    {
        perror("Failed to allocate memory for packet");
        exit(EXIT_FAILURE);
    }
    packet->start_marker = START_MARKER;
    packet->end_marker = END_MARKER;
    packet->origin = 0;
    packet->destination = 0;
    // memset(packet->origin, 0, 4);
    // memset(packet->destination, 0, 4);
    packet->card = 0;
    packet->type = 0;
    packet->receive_confirmation = 0;
    
    return packet;
}

// packet_t *create_or_modify_packet(packet_t *p, char *origin_addr, char *destination_addr, int card, int type)
packet_t *create_or_modify_packet(packet_t *p, int origin, int destination, int card, int type)
{

    // struct in_addr ip;
    
    if(p == NULL)
    {
        if((p = malloc(sizeof(packet_t))) == NULL)
        {
            perror("Failed to allocate memory for packet");
            exit(EXIT_FAILURE);
        }
    }
        memset(p, 0, sizeof(packet_t));
        p->card = START_MARKER;
        // memcpy(p->origin, &origin_addr->sin_addr, 4);
        // memcpy(p->destination, &destination_addr->sin_addr, 4);
        // if(inet_pton(AF_INET, origin_addr, &isuitp) != 1)
        // {
        //     fprintf(stderr, "Invalid origin IP address\n");
        //     free(p);
        //     exit(EXIT_FAILURE);
        // }
        // memcpy(p->origin, &ip, sizeof(ip));
        
        // if(inet_pton(AF_INET, destination_addr, &ip) != 1)
        // {
        //     fprintf(stderr, "Invalid destination IP address\n");
        //     free(p);
        //     exit(EXIT_FAILURE);
        // }
        // memcpy(p->destination, &ip, sizeof(ip));
        p->origin = origin;
        p->destination = destination;
        p->card = card;
        p->type = type;
        p->receive_confirmation = 0;
        p->end_marker = END_MARKER;

    return p;
}

int check_packet(packet_t *p)
{
    if(p->start_marker == START_MARKER && p->end_marker == END_MARKER)
        return 1;
    else
        return 0;
}


/* Verify what node has the token */
int has_token(network_t *net)
{
    if(net->token == 1)
        return 1;
    else
        return 0;
}

/* Send a packet to the next node */
int send_packet(network_t *net, packet_t *packet)
{
    int bytes_sent = sendto(net->socket_fd, packet, sizeof(packet_t), 0, (struct sockaddr *)&net->next_node_addr, sizeof(net->next_node_addr));
    if (bytes_sent < 0)
    {
        perror("Failed to send packet");
        return -3;
    } 
    else
    {
        // printf("Packet sent to %s:%d\n", inet_ntoa(net->next_node_addr.sin_addr), ntohs(net->next_node_addr.sin_port));
        return 0;
    }
}

int send_packet_and_wait(network_t *net, packet_t *response, packet_t *packet)
{
    if(packet == NULL)
    {
        perror("Packet is NULL");
        return -1;
    }

    send_packet(net, packet);

    while(1)
    {
        if(receive_packet(net, response) == 1)
        {
            response->receive_confirmation = 1;
            // printf("Received confirmation\n");
            return 1;
        }
        else
        {
            printf("Failed to receive confirmation\n");
            return 0;
        }
    }

}

/* Receive a packet from the current node */
int receive_packet(network_t *net, packet_t *packet)
{   
    socklen_t aux;
    aux = sizeof(net->current_node_addr);

    int recv_len = recvfrom(net->socket_fd, packet, sizeof(packet_t), 0, (struct sockaddr *)&net->current_node_addr, &aux);

    if (recv_len > 0)
    {
        packet->receive_confirmation = 1;
        // packet->origin = ntohl(packet->origin);
        // packet->destination = ntohl(packet->destination);
        // printf("Packet received from %s:%d\n", inet_ntoa(net->current_node_addr.sin_addr), htons(net->current_node_addr.sin_port));
        // printf("Packet received from %d:%d\n", packet->origin, packet->destination);
    } 
    else 
    {
        perror("Failed to receive packet");
        return -1;
    }
    
    return 1;
}

void uint8_to_ip_string(const uint8_t ip[4], char *str, size_t str_size) {
    struct in_addr ip_addr;
    memcpy(&ip_addr, ip, 4);

    if (inet_ntop(AF_INET, &ip_addr, str, str_size) == NULL) {
        perror("inet_ntop");
        exit(EXIT_FAILURE);
    }
}

int receive_packet_and_pass_forward(network_t *net)
{
    // packet_t *response = init_packet();
    receive_packet(net, net->packet);

    // char ip_str[INET_ADDRSTRLEN];
    // uint8_to_ip_string(net->packet->destination, ip_str, sizeof(ip_str));

    // if(strcmp(ip_str, net->players[net->node_id-1].ip) == 0)
    // {
    // 
    //         print_packet2(net->packet);
    //         printf("PACKET RECEIVED\n");
    //         net->packet->receive_confirmation = 1;
    //         send_packet(net, net->packet);
    //         return 1;
    // }
    if((int)net->packet->destination == net->players[net->node_id-1].id) {
            // print_packet(net->packet);
            // printf("PACKET RECEIVED\n");
            net->packet->receive_confirmation = 1;
            send_packet(net, net->packet);
            return 1;
    }
    else
    {
        // printf("Passing packet forward\n");
        send_packet(net, net->packet);
        return 0;
    }
}

/********************************************* Game Functions  **************************************************/

void init_deck_player(network_t *net)
{
    if((net->deck = malloc(sizeof(deck_t))) == NULL)
    {
        perror("Failed to allocate memory for deck");
        exit(EXIT_FAILURE);
    }

    if((net->deck->cards = malloc(sizeof(card_t) * NUM_PLAYER_CARDS)) == NULL)
    {
        perror("Failed to allocate memory for cards");
        exit(EXIT_FAILURE);
    }

    net->deck->size = 0;

    return;

}


deck_t *create_deck()
{

    deck_t *deck;
    // int ranks[SEQ_TOTAL] = {5, 4, 3, 12, 11, 10, 9, 8, 7, 6, 2, 1, 0};

    if((deck = (deck_t *)malloc(sizeof(deck_t))) == NULL)
    {
        perror("Failed to allocate memory for deck");
        exit(EXIT_FAILURE);
    }

    if((deck->cards = (card_t *)malloc(sizeof(card_t) * NUM_CARDS)) == NULL )
    {
        perror("Failed to allocate memory for cards");
        exit(EXIT_FAILURE);
    }

    deck->size = NUM_CARDS;

    for(int i = 0; i < NUM_CARDS; i++)
    {
        deck->cards[i].value = (i%SEQ_TOTAL) + 1;
        deck->cards[i].suit = i/SEQ_TOTAL;
        // deck->cards[i].rank = ranks[i%SEQ_TOTAL];
    }

    return deck;

}

int shuffle_deck(deck_t *deck)
{
    int i, j;
    card_t aux;

    for(i = 0; i < deck->size; i++)
    {
        j = rand() % deck->size;
        aux = deck->cards[i];
        deck->cards[i] = deck->cards[j];
        deck->cards[j] = aux;
    }

    return 1;
}


void distribute_cards(network_t *net, deck_t *deck)
{
    int cards_per_player = NUM_CARDS / net->num_nodes;
    int count = 0;
    uint8_t card_aux = 0;
    card_t card; 
    packet_t *packet = init_packet();
    packet_t *response = init_packet();

    for(int j = 0; j < cards_per_player; j++)
    {
        card = deck->cards[count];
        count++;
        net->deck->cards[j] = card;
        for(int k = 1; k < net->num_nodes; k++)
        {
            card = deck->cards[count];
            count++;
            print_card(card);
            
            set_card(&card_aux, card.value, card.suit);
            
            packet = create_or_modify_packet(packet, net->players[net->node_id-1].id, net->players[k].id, card_aux, SEND_CARD);
            /* deixar esta parter circular */
            send_packet_and_wait(net, response, packet);
        }
    }

    return;
}

void set_card(uint8_t *card, int value, int suit) {
     if (value < 0 || value > 13){
        printf("value1 deve estar entre 0 e 63\n");
        return;
    }
    if (suit < 0 || suit > 3) {
        printf("suit deve estar entre 0 e 3\n");
        return;
    }

    *card = (value & 0x3F) << 2 | (suit & 0x03);

}

void retrieve_card(uint8_t card, int *value, int *suit) {
    *value = (card >> 2) & 0x3F; // Extrai value1 dos 6 bits mais significativos
    *suit = card & 0x03;        // Extrai value2 dos 2 bits menos significativos
}

// uint8_t set_card(int suit, int value)
// {
//     uint8_t card = 0;

//     // Ensure suit and value are within valid ranges
//     if (suit < 0 || suit > 3 || value < 0 || value > 63) {
//         printf("Invalid suit or value\n");
//         return 0;
//     }

//     // Convert int to uint8_t to fit into card field
//     uint8_t suit_val = (uint8_t)suit;
//     uint8_t value_val = (uint8_t)value;

//     // Clear the relevant bits before setting new values
//     card &= ~CARD_SUIT_MASK;  // Clear the 2 bits for suit
//     card &= CARD_VALUE_MASK;  // Clear the 6 bits for value

//     // Set new suit and value
//     card |= (suit_val & CARD_SUIT_MASK);   // Set the 2 bits for suit
//     card |= (value_val & ~CARD_SUIT_MASK); // Set the 6 bits for value

//     return card;
// }

// int get_suit_bits(packet_t *packet) {
//     if (packet == NULL) {
//         return -1; // Handle NULL pointer cases
//     }
//     return (int)(packet->card & CARD_SUIT_MASK);
// }

// // Function to get value bits from the card field
// int get_value_bits(packet_t *packet) {
//     if (packet == NULL) {
//         return -1; // Handle NULL pointer cases
//     }
//     return (int)((packet->card & ~CARD_SUIT_MASK) >> 2); // Right shift to align bits
// }


void print_deck(deck_t *deck)
{
    for(int i = 0; i < deck->size; i++)
    {
        char *suit_symbol;
        char value;
        switch(deck->cards[i].suit)
        {
            case 0: suit_symbol = "♥"; break;
            case 1: suit_symbol = "♦"; break;
            case 2: suit_symbol = "♣"; break;
            case 3: suit_symbol = "♠"; break;
            default: suit_symbol = "?"; break;
        }
        switch(deck->cards[i].value)
        {
            case 1: value = 'A'; break;
            case 11: value = 'J'; break;
            case 12: value = 'Q'; break;
            case 13: value = 'K'; break;  
        }

        if(deck->cards[i].value > 1 && deck->cards[i].value < 11)
            printf("Card %d: %d of %s\n", i+1, deck->cards[i].value, suit_symbol);
        else
            printf("Card %d: %c of %s\n", i+1, value , suit_symbol);
    }
}

void print_deck_player(deck_t *deck)
{
    for(int i = 0; i < deck->size; i++)
    {
        char *suit_symbol;
        char value;
        switch(deck->cards[i].suit)
        {
            case 0: suit_symbol = "♥"; break;
            case 1: suit_symbol = "♦"; break;
            case 2: suit_symbol = "♣"; break;
            case 3: suit_symbol = "♠"; break;
            default: suit_symbol = "?"; break;
        }
        switch(deck->cards[i].value)
        {
            case 1: value = 'A'; break;
            case 11: value = 'J'; break;
            case 12: value = 'Q'; break;
            case 13: value = 'K'; break;  
        }

        if(deck->cards[i].value > 1 && deck->cards[i].value < 11)
            printf("%d%s ", deck->cards[i].value, suit_symbol);
        else
            printf("%c%s ", value , suit_symbol);
        
        if(i % 4 == 0)
            printf("\n");
    }
}

void print_card(card_t card)
{
    char *suit_symbol;
    char value;
    
    switch(card.suit)
    {
        case 0: suit_symbol = "♥"; break;
        case 1: suit_symbol = "♦"; break;
        case 2: suit_symbol = "♣"; break;
        case 3: suit_symbol = "♠"; break;
        default: suit_symbol = "?"; break;
    }
    switch(card.value)
    {
        case 1: value = 'A'; break;
        case 11: value = 'J'; break;
        case 12: value = 'Q'; break;
        case 13: value = 'K'; break;  
    }

    if(card.value > 1 && card.value < 11)
        printf("Card: %d of %s\n", card.value, suit_symbol);
    else
        printf("Card: %c of %s\n", value , suit_symbol);
}
