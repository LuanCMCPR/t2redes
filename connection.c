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
            // ID matches, no need to continue reading
            fclose(file);
            return;
        }
        count++;
    }
}

void init_network(network_t *net)
{
    if(net->token)
    {
        printf("I node %d have the token\n", net->node_id);
        printf("Sending start packet\n");

        while(1)
        {
            net->packet = create_or_modify_packet(NULL, net->node_id, net->node_id, 0, INIT_NETWORK);
            packet_t *response = create_or_modify_packet(NULL, 0, 0, 0, 0);
        
            send_packet_and_wait(net, response, net->packet);
            if(response->receive_confirmation == 1)
            {
                printf("Received confirmation\n");
                break;
            }
    
            // if(response)
            // {
            //     printf("Received response.Everyone is ready\n");
            //     int check_result = is_all_ready(response, net->num_nodes, net->node_id);
            //     if(check_result == -1)
            //     {
            //         free(response);
            //         break;
            //     } 
            //     else
            //     {
            //         printf("Player %d is not ready\n", check_result);
            //         break;
            //     }
            //     free(response);
            // } 
            // else
            // {
            //     printf("No response received\n");
            // }
        }
    }
    else
    {
        printf("I don't have the token, waiting for start packet\n");
        net->packet = create_or_modify_packet(NULL, 0, 0, 0, 0);
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

packet_t *create_or_modify_packet(packet_t *p, int origin, int destination, int card, int type)
{
    if(p == NULL)
    {
        if((p = malloc(sizeof(packet_t))) == NULL)
        {
            perror("Failed to allocate memory for packet");
            exit(EXIT_FAILURE);
        }
        p->start_marker = START_MARKER;
        p->origin = origin;
        p->destination = destination;
        p->card = card;
        p->type = type;
        p->receive_confirmation = 0;
        p->end_marker = END_MARKER;
    }
    else
    {
        p->origin = origin;
        p->destination = destination;
        p->card = card;
        p->type = type;
        p->receive_confirmation = 0;
    }
    return p;
}


/* Verify what node has the token */
int has_token(network_t *net)
{
    return net->token;
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
        printf("Packet sent to %s:%d\n", inet_ntoa(net->next_node_addr.sin_addr), ntohs(net->next_node_addr.sin_port));
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
        receive_packet(net, response);
        if(response->receive_confirmation == 1)
        {
            printf("Received confirmation\n");
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
        // printf("Packet received from %s:%d\n", inet_ntoa(sender_addr.sin_addr), ntohs(sender_addr.sin_port));
        printf("Packet received from %d:%d\n", packet->origin, packet->destination);
    } 
    else 
    {
        perror("Failed to receive packet");
        return -1;
    }
    
    return 1;
}



/********************************************* NOT IN USE  **************************************************/
/********************************************* NOT IN USE  **************************************************/
/********************************************* NOT IN USE  **************************************************/
char *get_current_node_info(node_t *players, int num_players, int *port)
{
    int temp_socket = create_socket();

    struct sockaddr_in temp_addr;
    memset(&temp_addr, 0, sizeof(temp_addr));
    temp_addr.sin_family = AF_INET;
    temp_addr.sin_port = htons(1);
    temp_addr.sin_addr.s_addr = inet_addr("8.8.8.8");

    if(connect(temp_socket, (struct sockaddr *)&temp_addr, sizeof(temp_addr)) < 0)
    {
        close(temp_socket);
        perror("Failed to connect to Google DNS");
        exit(EXIT_FAILURE);
    }

    struct sockaddr_in local_addr;
    socklen_t addr_len = sizeof(local_addr);
    if(getsockname(temp_socket, (struct sockaddr *)&local_addr, &addr_len) < 0)
    {
        close(temp_socket);
        perror("Failed to get local address");
        exit(EXIT_FAILURE);
    }

    char *address = inet_ntoa(local_addr.sin_addr);
    printf("This machine IP: %s\n", address);
    *port = get_my_port(players, num_players, address);

    close(temp_socket);
    return address;
}

int get_my_id(node_t *players, int num_players, char *address)
{
    for(int i = 0; i < num_players; i++)
    {
        if(strcmp(players[i].ip, address) == 0)
            return players[i].id;
    }
    perror("Couldn't find my id");
    return -1;
}


int get_my_port(node_t *players, int num_players, char* address )
{
    for(int i = 0; i < num_players; i++)
    {
        if(strcmp(players[i].ip, address) == 0)
            return players[i].port;
    }
    perror("Couldn't find my port");
    return -2;

}

char *get_next_node_info(node_t *players, int num_players, int *port)
{
    for(int i = 0; i < num_players; i++)
    {
        if(strcmp(players[i].ip, (players[(i+1) % num_players]).ip) == 0)
        {
            *port = players[(i + 1) % num_players].port;
            return players[(i + 1) % num_players].ip;
        }
    }
    perror("Couldn't find node info");
    return NULL;
}

/********************************************* NOT IN USE  **************************************************/
/********************************************* NOT IN USE  **************************************************/
/********************************************* NOT IN USE  **************************************************/
// node_t *get_next_node(network_t *net)
// {
//     for(int i = 0; net->players; i++)
//         strcmp(net->next_node_addr, net->players[i].ip)
// }

/********************************************* NOT IN USE  **************************************************/

int is_first_node(node_t *players, int num_players)
{   
    for(int i=0; i < num_players; i++)
    {
        if(players[i].id == 0)
            return players[i].id;
    }
    return -6;

}

int mark_packet_as_received(packet_t *p)
{
    if(p->receive_confirmation == 1)
        return 1;
    else
        return 0;
}

int receive_packet_and_pass_forward(network_t *net)
{
    return net->node_id;
}




/* Send the token to the next node */
// void send_token(network_t *net)
// {
//     node_t next_node = get_next_node(net);
//     packet_t token;
//     // packet_t token_packet = create_packet(net->node_id, next_node.id, 0, 0, 0);
//     send_packet(net, &token_packet);
//     net->token = 0;

//     return;
// }

int is_all_ready(packet_t *response, int num_nodes, int node_id)
{
    if(response != NULL)
    {
        for(int i = 0; i < num_nodes; i++)
        {
            if(response->origin != node_id)
            {
                return response->origin;
            }
        }
    }
    return -1;
}

/********************************************* NOT IN USE  **************************************************/