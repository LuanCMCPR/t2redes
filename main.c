#include "connection.h"
#include <stdio.h>
#define MAX_PLAYERS 4

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


    printf("Network Info\n");
    network_t *net = network_config(players, MAX_PLAYERS, index);
    print_network(net);
    printf("\n\n");

    printf("Node info\n");
    print_node(&players[index]);
    free(net);

    return 0;
}
