#include "connection.h"
#include <stdio.h>

int main(int argc, char *argv[])
{
    if(argc != 3)
    {
        fprintf(stderr, "Usage: %s <config_file> <index>\n", argv[0]);
        exit(EXIT_FAILURE);
    }
    int index = atoi(argv[2]) - 1;

    if(index+1 > 4 || index + 1 < 1)
    {
        fprintf(stderr, "Min id: %d\nMax id: %d\n", 1, MAX_PLAYERS);
        exit(EXIT_FAILURE);

    }
        

    node_t players[MAX_PLAYERS];
    load_config(argv[1], players, MAX_PLAYERS);
    deck_t *deck = create_deck();

    // printf("Network Info\n");
    network_t *net = network_config(players, MAX_PLAYERS, index);
    // print_network(net);
    // printf("\n\n");

    // printf("Node info\n");
    // print_node(&players[index]);

    // char *message = "Hello man";
    // int msg_len = strlen(message);
    // char message_buffer[1024];

    // socklen_t len = sizeof(net->next_node_addr);

    // memcpy(message_buffer, &msg_len, sizeof(int));
    // memcpy(message_buffer + sizeof(int), message, msg_len);

    // sendto(net->socket_fd, message_buffer, sizeof(int) + msg_len, MSG_CONFIRM, (const struct sockaddr *)&net->next_node_addr, len);
    // printf("Message sent to next: %s\n", message);

    // char buffer[1024];
    // int n;
    // // Recebendo mensagem do computador anterior
    // n = recvfrom(net->socket_fd, buffer, 1024, MSG_WAITALL, (struct sockaddr *)&net->current_node_addr, &len);

    // int recv_msg_len;
    // memcpy(&recv_msg_len, buffer, sizeof(int));
    // char recv_message[recv_msg_len + 1];
    // memcpy(recv_message, buffer + sizeof(int), recv_msg_len);
    // recv_message[recv_msg_len] = '\0';

    // printf("Message received from previous: %s\n", recv_message);

    /* Init Network */

    /* Baralho */

    init_network(net);  
    
    if(has_token(net))
    {
        printf("Press enter to start the game\n");
        getchar();
        // printf("Before shuffle\n");
        // print_deck(deck);
        // shuffle_deck(deck);
        // printf("After shuffle\n");
        // print_deck(deck);
        distribute_cards(net, deck);
    }
    else
    {
        receive_packet_and_pass_forward(net);
        if(net->packet->type == SEND_CARD)
        {
            printf("Received card\n");
            printf("Card: %d\n", net->packet->card);
            
        }
        else
            printf("Waiting cards\n");

    }
    
    /*Jogo Foda-se ou fodinha */

    // while(1)
    // {
    //     if(has_token(net))
    //     {
    //         distribute_cards(net);
    //     }
    //     else
    //     {
    //         printf("I don't have the token\n");
    //         packet_t *p = (packet_t *)malloc(sizeof(packet_t));
    //         receive_packet(net, p);
    //         printf("Token received\n");
    //         net->token = 1;
    //     }
    // }

    close(net->socket_fd);
    free(net);

    return 0;
}
