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

    network_t *net = network_config(players, MAX_PLAYERS, index);

    system("clear");

    // init_network(net);  

    printf("Waiting Start the game\n");
    getchar();
    deck_t *deck = create_deck();    

    while(net->round < NUM_ROUNDS)
    {
        
        /* Distribute cards */
        distribute_cards(net, deck);

        /* Predicitions */
        predictions(net);

        /* Play cards */
        play_round(net);

        /* Check winner */
        end_round(net);
        printf("DENTRO LOOP");
    }

    match_end(net);

    close(net->socket_fd);
    free(net);

    return 0;
}




    // /*  Card dealer always begin with token */
    // if(net->node_id == net->card_dealer)
    // {
    //     printf("Press enter to start the game\n");
    //     getchar();
    //     distribute_cards(net, deck);
    //     printf("Cards distributed by player: %d\n", net->node_id);
    //     make_prediction(net);
    //     // create_or_modify_packet(net->packet, net->node_id, (net->node_id + 1) % (MAX_PLAYERS), 0, PREDCTION);
    //     // send_packet(net, net->packet);
    //     net->packet->type = 0;
    //     pass_token(net);
    // }

    // while(1)
    // {
    //     /* Receive Cards */
    //     receive_packet_and_pass_forward(net);
    //     if(net->packet->type == SEND_CARD && net->packet->destination == net->node_id)
    //     {
    //         // printf("Received card\n");
    //         int value, suit;    
    //         retrieve_card(net->packet->card, &value, &suit);
    //         net->deck->cards[net->deck->size].suit = suit;
    //         net->deck->cards[net->deck->size].value = value;
    //         print_card(net->deck->cards[net->deck->size]);
    //         net->deck->size++;
    //         if(net->deck->size == NUM_PLAYER_CARDS)
    //         {
    //             printf("Player %d received all cards\n", net->node_id);
    //             net->packet->type = 0;
    //         }    
    //     }
    //     else if(net->packet->type == SEND_TOKEN && net->packet->destination == net->node_id)
    //     {
    //         net->token = 1;
    //         printf("Player %d received token\n", net->node_id);
    //         net->packet->type = 0;
    //     }
    //     if(has_token(net))
    //     {
                
    //         if(net->node_id == 1)
    //         {
    //             printf("Starting new round\n");
    //             play_round(net);
    //             break;
    //         }
    //         make_prediction(net);
    //         net->packet->type = 0;
    //         pass_token(net);
    //         break;

    //     }
    // }

        


