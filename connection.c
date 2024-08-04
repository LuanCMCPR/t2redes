#include "connection.h"


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
    net->card_dealer = 1;
    net->round = 1;
    net->last_winner = -1;
    net->game_phase = DISTRIBUTE_CARDS;
    memset(net->predictions, 0, sizeof(net->predictions));
    memset(net->lifes, 13, sizeof(net->lifes));
    memset(net->score, 0, sizeof(net->score));
    net->packet = init_packet();
    init_deck_player(net);


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

    if (strcmp(players[index].ip, players[index].next_ip) == 0)
        if(players[index].port == players[index].next_port)
        {
            perror("I'm the only node in the network");
            return NULL;
        }

    if(players[index].id == 1)
        net->token = 1;
    else
        net->token = 0;


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
                net->packet->receive_confirmation = 1;
                send_packet(net, NULL);
                break;
            }

            if (net->packet->type == INIT_NETWORK)
            {
                printf("Received init packet again. Passing forward...\n");
                net->packet->receive_confirmation = 1;
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
    packet->type = 0;
    memset(packet->data, 0, sizeof(packet->data));
    packet->receive_confirmation = 0;
    
    return packet;
}

// packet_t *create_or_modify_packet(packet_t *p, char *origin_addr, char *destination_addr, int card, int type)
packet_t *create_or_modify_packet(packet_t *p, int origin, int destination, uint8_t *data, int type)
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
    p->start_marker = START_MARKER;
    p->origin = origin;
    p->destination = destination;
    memcpy(p->data, data, sizeof(p->data));
    p->type = type;
    p->receive_confirmation = 0;
    p->end_marker = END_MARKER;

    return p;
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
    sendto(net->socket_fd, packet, sizeof(packet_t), 0, (struct sockaddr *)&net->next_node_addr, sizeof(net->next_node_addr));
    return 1;
}


int send_packet_and_wait(network_t *net, packet_t *response, packet_t *packet)
{

    send_packet(net, packet);

    while(receive_packet(net, response))
    {
        switch (response->type)
        {
        case SEND_CARD:
            break;

        case MAKE_PREDICTION:
            for(int i = 0; i < net->num_nodes; i++)
                net->predictions[i] = response->data[i];
            break;
        case SHOW_PREDICTION:
            for(int i = 0; i < net->num_nodes; i++)
                printf("Player %d predicted %d\n", i+1, response->data[i]);
            break;
        case PLAY_CARD:
            show_played_card(net);
            net->last_winner = calculate_results(net);
            break;
        case END_ROUND:
            net->card_dealer = (net->card_dealer + 1) % net->num_nodes;
            break;
        case END_MATCH:
            printf("Match ended\n");
            break;
        default:    
            break;
        }
        if(response->receive_confirmation == 1)
            return 1;
    }
    return 1;

}

/* Receive a packet from the current node */
int receive_packet(network_t *net, packet_t *packet)
{   
    socklen_t aux;
    aux = sizeof(net->current_node_addr);

    while(!recvfrom(net->socket_fd, packet, sizeof(packet_t), 0, (struct sockaddr *)&net->current_node_addr, &aux));

    return 1;
}

int receive_packet_and_pass_forward(network_t *net)
{
    while(!receive_packet(net, net->packet));

    int target_node = (net->node_id % net->num_nodes); 

    switch (net->packet->type)
    {
        if(net->deck->size == NUM_CARDS + 1 - net->round)
            break;
        int value, suit;
        uint8_t data[4];    
        case SEND_CARD:
            retrieve_card(net->packet->data[0], &value, &suit);
            net->deck->cards[net->deck->size].suit = suit;
            net->deck->cards[net->deck->size].value = value;
            net->deck->size++;
        break;
        case MAKE_PREDICTION:
            memset(data, 0, sizeof(data));
            data[net->node_id-1] = calculate_prediction(net);
            create_or_modify_packet(net->packet, net->card_dealer, net->players[target_node].id, data, MAKE_PREDICTION);
        break;

        case SHOW_PREDICTION:
            for(int i = 0; i < net->num_nodes; i++)
                printf("Player %d predicted %d\n", i+1, net->packet->data[i]);
        break;
        
        case PLAY_CARD:
            memset(data, 0, sizeof(data));
            show_played_card(net);
            card_t card = get_card(net->deck);
            set_card(&data[net->node_id-1], card.value, card.suit);
            create_or_modify_packet(net->packet, net->card_dealer, net->players[target_node].id, data, PLAY_CARD);
        break;

        case END_ROUND:
            show_round_results(net);
            net->last_winner = net->packet->data[0]; 
            net->card_dealer = (net->card_dealer + 1) % net->num_nodes;
            net->round++;
        break;

        default:

        break;
    }

    if(net->packet->destination == net->node_id)
    {
        net->packet->receive_confirmation = 1;
    }
    
    send_packet(net, net->packet);

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


/********************************************* Game Functions  **************************************************/


void match_end(network_t *net)
{
    uint8_t data[4];
    printf("Match ended\n");
    for(int i = 0; i < net->num_nodes; i++)
    {
        net->lifes[i] = net->lifes[i] - abs(net->predictions[i] - net->score[i]);
    }
    for(int i = 0; i < net->num_nodes; i++)
    {
        printf("Player %d lifes: %d\n", i+1, net->lifes[i]);
    }
    
    int max_life = -1;
    int player = -1;
    for(int i = 0; i < net->num_nodes; i++)
    {
        if(net->lifes[i] > max_life)
        {
            max_life = net->lifes[i];
            player = i+1;
        }
    }

    printf("Player %d wins the match\n", player);

    data[0] = player;
    int target_node = (net->node_id) % net->num_nodes;
    create_or_modify_packet(net->packet, net->node_id, net->players[target_node].id, data, END_MATCH);

    send_packet_and_wait(net, net->packet, net->packet);
}

void end_round(network_t *net)
{
    uint8_t data[4];
    int target_node = (net->node_id) % net->num_nodes;
    data[0] = net->last_winner;
    create_or_modify_packet(net->packet, net->node_id, target_node, data, END_ROUND);
    net->score[net->last_winner-1]++;
    send_packet_and_wait(net, net->packet, net->packet);
    net->round++;
    pass_token(net);
}


void play_round(network_t *net)
{
    uint8_t data[4];
    if(net->node_id == net->card_dealer)
    {
        card_t card = get_card(net->deck);
        set_card(&data[net->node_id-1], card.value, card.suit);
        int target_node = (net->node_id % net->num_nodes);
        create_or_modify_packet(net->packet, net->players[net->node_id-1].id, net->players[target_node].id, data, PLAY_CARD);
        send_packet_and_wait(net, net->packet, net->packet);
    }
    else
        receive_packet_and_pass_forward(net);
        

}

void show_round_results(network_t *net)
{
    printf("Round %d results\n", net->round);
    printf("Player %d wins the round\n", net->packet->data[0]);

    // for(int i = 0; i < net->num_nodes; i++)
    // {
    //     printf("Player %d score: %d\n", i+1, net->score[i]);
    // }

}

int calculate_results(network_t *net)
{
    card_t card;
    card_t max;
    int winner = -1;

    max.value = 0;
    max.suit = 0;

    for(int i = 0; i < net->num_nodes; i++)
    {
        retrieve_card(net->packet->data[i], &card.value, &card.suit);
        if(card.value > max.value)
        {
            max.value = card.value;
            max.suit = card.suit;
            winner = i+1;
        }
        else if(card.value == max.value)
        {
            if(card.suit > max.suit)
            {
                max.value = card.value;
                max.suit = card.suit;
                winner = i+1;
            }
        }        
    }

    return winner;
}

/* Get a card of the deck to play */
card_t get_card(deck_t *deck)
{
    card_t card;
    card = deck->cards[0];

    for(int i = 0; i < deck->size-1; i++)
        deck->cards[i] = deck->cards[i+1];

    deck->size--;

    return card;
}

/* Show player cards of the round */
void show_played_card(network_t *net)
{
    card_t card;

    for(int i = 0; i < net->node_id -1; i++)
    {
        printf("Player %d played ", i+1);
        retrieve_card(net->packet->data[i], &card.value, &card.suit);
        print_card(card);
    }
}

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
    if(!has_token(net))
        

    shuffle_deck(deck);

    int cards_per_player = NUM_CARDS / net->num_nodes;
    int count = 0;
    card_t card; 
    packet_t *packet = init_packet();
    packet_t *response = init_packet();
    uint8_t data[4];

    for(int j = 0; j < cards_per_player; j++)
    {
        card = deck->cards[count];
        count++;
        net->deck->cards[j] = card;
        net->deck->size++;
        for(int k = 1; k < net->num_nodes; k++)
        {
            int target_node = (net->node_id - 1 + k) % net->num_nodes; 
            card = deck->cards[count];
            count++;
            
            set_card(&data[0], card.value, card.suit);
            
            packet = create_or_modify_packet(packet, net->players[net->node_id-1].id, net->players[target_node].id, data, SEND_CARD);
            send_packet_and_wait(net, response, packet);
        }
    }

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

void print_card(card_t card)
{

    char *suit_symbol;
    char value, aux;

    switch(card.suit)
    {
        case 3: suit_symbol = "♣"; break;
        case 2: suit_symbol = "♥"; break;
        case 1: suit_symbol = "♠"; break;
        case 0: suit_symbol = "♦"; break;
        default: suit_symbol = "?"; break;
    }
    switch(card.value)
    {
        case 1: value = '4'; break;
        case 2: value = '5'; break;
        case 3: value = '6'; break;
        case 4: value = '7'; break;
        case 5: value = '8'; break;
        case 6: value = '9'; break;
        case 7: value = '1', aux = '0'; break;
        case 8: value = 'J'; break;
        case 9: value = 'Q'; break;
        case 10: value = 'K'; break;
        case 11: value = 'A'; break;
        case 12: value = '2'; break;
        case 13: value = '3'; break;  
    }

    if(card.value == 7)
        printf("%c%c%s ", value, aux, suit_symbol);
    else
        printf("%c%s ", value , suit_symbol);
    
}

int calculate_prediction(network_t *net)
{
    int prediction = 1;

    for(int i = 0; i < net->deck->size; i++)
    {
        if(net->deck->cards[i].value >= 9)
            prediction++;
    }

    if(prediction > 1 )
        return prediction - 1;

    return prediction;
}

void predictions(network_t *net)
{

    if(net->node_id == net->card_dealer)
    {

        net->predictions[net->node_id-1] = calculate_prediction(net);
        int target_node = (net->node_id % net->num_nodes);
        create_or_modify_packet(net->packet, net->players[net->node_id-1].id, net->players[target_node].id, net->predictions, MAKE_PREDICTION);
        send_packet_and_wait(net, net->packet, net->packet);
        create_or_modify_packet(net->packet, net->players[net->node_id-1].id, net->players[target_node].id, net->predictions, SHOW_PREDICTION);
        send_packet_and_wait(net, net->packet, net->packet);
        return;
    }
    else
        receive_packet_and_pass_forward(net);
        

}

void pass_token(network_t *net)
{

    net->token = 0;
    printf("Passing token to player %d\n", net->node_id+1 % net->num_nodes);
    create_or_modify_packet(net->packet, net->players[net->node_id-1].id, net->players[net->node_id % net->num_nodes].id, 0, SEND_TOKEN);
    send_packet(net, net->packet);

}