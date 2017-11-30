#include <stdio.h>
#include "toychess.c"

Move human_mover(Bitboard board)
{
    // Mover implementation for a real human player
    char inbuff[20];
    char algebra[20];
    char *help_buffer;
    int idx;
    Move result = {};
    while(result.dst == EMPTY_BOARD) {
        printf("\nEnter your move > ");
        fgets(inbuff, 20, stdin);
        if(strcmp(inbuff, "help\n") == 0) {
            // give the player a hand and list available moves
            Move *move_list = legal_moves_for_board(board);
            idx = 1;
            while(move_list != NULL) {
                help_buffer = algebra_for_move(board, *move_list);
                printf("%s", help_buffer);
                if(idx % 5 == 0) {
                    printf("\n");
                } else {
                    printf("\t");
                }
                free(help_buffer);
                move_list = move_list->next;
                idx ++;
            }
            printf("\n\n");
            print_board(board);
        } else {
            strncpy(algebra, inbuff, strlen(inbuff) - 1);
            algebra[(int)strlen(inbuff) - 1] = 0;
            result = parse_algebra(board, algebra);
        }
    }
    return result;
}


int main()
{
    printf("****************\nWELCOME TO CHESS\n****************\n\n");
    printf("Human plays black. Input is (almost) PGN standard algebraic\n");
    printf("notation\n\nType 'help' to list available moves.\n\n");
    match_player(negamax_mover, human_mover);
}
