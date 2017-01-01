#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include "toychess.h"


/* const bitmasks for bitboard */
const uint64_t FILE_A = (uint64_t)0x8080808080808080;
const uint64_t FILE_H = (uint64_t)0x0101010101010101;
const uint64_t FILE_AB = (uint64_t)0xC0C0C0C0C0C0C0C0;
const uint64_t FILE_GH = (uint64_t)0x0303030303030303;
const uint64_t RANK_8 = (uint64_t)0x00000000000000FF;
const uint64_t LOWEST_SQUARE = (uint64_t)0x0000000000000001;
const uint64_t SQUARE_0 = (uint64_t)0x8000000000000000;
const uint64_t EMPTY_BOARD = (uint64_t)0x0000000000000000;


void print_board(int *board)
{
    /* print a board to the terminal. expects 8x8 format, taking bottom right
     * as square 0 */
    printf("\e[39m\e[49m");
    printf(" ABCDEFGH\n");
    int file;
    int rank;
    for (rank=0; rank<8; rank++) {
        printf("\e[39m\e[49m");
        printf("%d", 8 - rank);
        for (file=0; file<8; file++) {
            /* choose square background color */
            if ((rank + file) % 2 == 1) {
                printf("\e[45m");
            } else {
                printf("\e[46m");
            }
            int piece;
            piece = board[63 - ((rank * 8) + (7-file))];
            if (piece & WHITE) {
                printf("\e[97m");
            } else {
                printf("\e[30m");
            }
            printf("%c", piece_letter(piece));
        }
        printf("\e[39m\e[49m");
        printf("\n");
    }
}

char piece_letter(int piece) 
{
    int uncoloured_piece = piece & ~WHITE;
    switch(uncoloured_piece) {
        case EMPTY:
            return ' ';
        case PAWN:
            return 'p';
        case KNIGHT:
            return 'N';
        case BISHOP:
            return 'B';
        case ROOK:
            return 'R';
        case QUEEN:
            return 'Q';
        case KING:
            return 'K';
    }
}


struct bitboard* new_board() 
{
    struct bitboard* board = (struct bitboard* )malloc( sizeof(struct bitboard) );
    /* intitialise everyhing to empty */
    uint64_t empty_row = (uint64_t)0x0000000000000000;
    board->whites = empty_row;
    board->moved = empty_row;
    board->pawns = empty_row;
    board->rooks = empty_row;
    board->knights = empty_row;
    board->bishops = empty_row;
    board->kings = empty_row;
    board->pawns = empty_row;
}


void populate_board( struct bitboard * board )
{
    /*put some pieces on the board*/
    board->pawns = (uint64_t)0x00FF00000000FF00;
    board->rooks = (uint64_t)0x8100000000000081;
    board->knights = (uint64_t)0x4200000000000042;
    board->bishops = (uint64_t)0x2400000000000024;
    board->kings = (uint64_t)0x0800000000000008;
    board->queens = (uint64_t)0x1000000000000010;
    board->whites = (uint64_t)0xFFFF000000000000;
}


void rotate_board_180( struct bitboard * board )
{
    /*rotate the entire board 180 degrees*/
    board->pawns = rotate_180(board->pawns);
    board->rooks = rotate_180(board->rooks);
    board->knights = rotate_180(board->knights);
    board->bishops = rotate_180(board->bishops);
    board->kings = rotate_180(board->kings);
    board->queens = rotate_180(board->queens);
    board->whites = rotate_180(board->whites);
}


int * to_8x8( struct bitboard * board )
{
    /* convert bit board into 8x8 hex format for display */
    int *board_8x8 = malloc(sizeof(int) * 64);
    uint64_t target_piece = SQUARE_0;
    int i;
    for (i=0; i < 64; i++) {
        int piece = 0;
        /*target_piece = (uint64_t)0x01 << i;*/
        if (board->pawns & target_piece) {
            piece = PAWN;
        }  else if (board->rooks & target_piece) {
            piece = ROOK;
        }  else if (board->knights & target_piece) {
            piece = KNIGHT;
        }  else if (board->bishops & target_piece) {
            piece = BISHOP;
        }  else if (board->queens & target_piece) {
            piece = QUEEN;
        }  else if (board->kings & target_piece) {
            piece = KING;
        }
        if (board->whites & target_piece) {
            piece = piece | WHITE;
        }
        board_8x8[i] = piece;
        target_piece >>= 1;
    }
    return board_8x8;
}

int population_count ( uint64_t bitboard_layer )
{
    /* count the set bits */
    int c;
    for (c = 0; bitboard_layer; bitboard_layer >>= 1) {
        c += bitboard_layer & LOWEST_SQUARE;
    }
    return c;
}


uint64_t occupied_squares( struct bitboard * board )
{
    /* any square with a piece on it */
    return board->pawns | board->rooks | board->knights | board->bishops | board->kings | board->queens;
}


uint64_t pawn_moves( struct bitboard * board )
{
    /* pawn moves available in open play */
    uint64_t occupied = occupied_squares(board);
    uint64_t moved_pawns = board->pawns & board->whites & board->moved;
    uint64_t unmoved_pawns = board->pawns & board->whites & ~board->moved;
    /*move unmoved pawns and merge with moved*/
    moved_pawns = moved_pawns | (((unmoved_pawns & ~RANK_8) << 8) & ~occupied);
    moved_pawns = moved_pawns | (((moved_pawns & ~RANK_8) << 8) & ~occupied);
    return moved_pawns;
}


uint64_t rook_attacks(uint64_t rooks, uint64_t enemies, uint64_t allies)
{
    /* sliding attack to the north, south, east and west */
    uint64_t moves = sliding_attack(shift_n, rooks, enemies, allies);
    moves |= sliding_attack(shift_e, rooks, enemies, allies);
    moves |= sliding_attack(shift_s, rooks, enemies, allies);
    return moves | sliding_attack(shift_w, rooks, enemies, allies);
}


uint64_t bishop_attacks(uint64_t bishops, uint64_t enemies, uint64_t allies)
{
    uint64_t moves = sliding_attack(shift_ne, bishops, enemies, allies);
    moves |= sliding_attack(shift_nw, bishops, enemies, allies);
    moves |= sliding_attack(shift_se, bishops, enemies, allies);
    return moves | sliding_attack(shift_sw, bishops, enemies, allies);
}


uint64_t queen_attacks(uint64_t queens, uint64_t enemies, uint64_t allies)
{
    /* queen attacks == bishop_attacks | rook_attacks */
    uint64_t moves = bishop_attacks(queens, enemies, allies);
    return moves | rook_attacks(queens, enemies, allies);
}


uint64_t king_attacks(uint64_t kings, uint64_t allies)
{
    uint64_t moves = shift_n(kings) | shift_ne(kings) | shift_e(kings);
    moves |= shift_se(kings) | shift_s(kings) | shift_sw(kings);
    moves |= shift_w(kings) | shift_nw(kings);
    return moves & ~allies;
    
}


uint64_t knight_attacks(uint64_t knights, uint64_t allies)
{
/*
 * Shift Knights to possible destinations, we don't care about enemies, but
 * can't land on an ally. Compass from chessprogramming wiki
 *
 *         noNoWe    noNoEa
 *             +15  +17
 *              |     |
 * noWeWe  +6 __|     |__+10  noEaEa
 *               \   /
 *                >0<
 *            __ /   \ __
 * soWeWe -10   |     |   -6  soEaEa
 *              |     |
 *             -17  -15
 *          soSoWe    soSoEa
*/
    uint64_t moves = (knights & ~FILE_H) >> 17;
    moves |= (knights & ~FILE_GH) >> 10;
    moves |= (knights & ~FILE_GH) << 6;
    moves |= (knights & ~FILE_H) << 15;
    moves |= (knights & ~FILE_A) << 17;
    moves |= (knights & ~FILE_AB) << 10;
    moves |= (knights & ~FILE_AB) >> 6;
    moves |= (knights & ~FILE_A) >> 15;
    return moves & ~allies;
}


uint64_t pawn_attacks(uint64_t pawns, uint64_t allies)
{
    /* attack to the ne or nw */
    uint64_t moves = shift_ne(pawns) | shift_nw(pawns);
    return moves & ~allies;
}


uint64_t sliding_attack(
    uint64_t (*slider)(uint64_t),
    uint64_t attackers,
    uint64_t enemies,
    uint64_t allies) 
{
    /* move attacking pieces with slider. Stop at allies, or after
     * enemies */
    uint64_t moves = (uint64_t) 0x00;
    while (attackers) {
        attackers=slider(attackers);
        moves |= attackers & ~allies;
        attackers = attackers & ~(allies | enemies);
    }
    return moves;
}


uint64_t shift_n( uint64_t bitlayer )
{
    return bitlayer >> 8;
}


uint64_t shift_ne( uint64_t bitlayer )
{
    return (bitlayer & ~FILE_A) >> 7;
}


uint64_t shift_nw( uint64_t bitlayer )
{
    return (bitlayer & ~FILE_H) >> 9;
}


uint64_t shift_s( uint64_t bitlayer )
{
    return bitlayer << 8;
}


uint64_t shift_e( uint64_t bitlayer ) 
{
    return (bitlayer & ~FILE_A) << 1;
}


uint64_t shift_se( uint64_t bitlayer )
{
    return (bitlayer & ~FILE_A) << 7;
}


uint64_t shift_sw( uint64_t bitlayer )
{
    return (bitlayer & ~FILE_H) << 9;
}


uint64_t shift_w( uint64_t bitlayer )
{
    return (bitlayer & ~FILE_H) >> 1;
}


uint64_t rotate_180( uint64_t bitlayer )
{
    /* Adapted from
     * https://graphics.stanford.edu/~seander/bithacks.html#BitReverseObvious
     */
    uint64_t reversed = bitlayer;
    uint64_t lsb = (uint64_t)0x01; /* cast to uint64 */
    int bits_left = 63;
    for (bitlayer >>= 1; bitlayer; bitlayer >>= 1) {
        reversed <<= 1;
        reversed |= bitlayer & lsb;
        bits_left --;
    }
    return reversed << bits_left;
}


uint64_t sq_bit(char file, int rank)
{
    /*
     * convert a rank and file into a bitmap with a single bit set
     * file a-h (lower case), rank 1-8
     */
    int file_int = file - 97;
    rank --;
    int position = 8 * rank + file_int;
    return SQUARE_0 >> position;
}
