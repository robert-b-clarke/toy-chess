#include<stdint.h>

enum squares {
    a1, b1, c1, d1, e1, f1, g1, h1,
    a2, b2, c2, d2, e2, f2, g2, h2,
    a3, b3, c3, d3, e3, f3, g3, h3,
    a4, b4, c4, d4, e4, f4, g4, h4,
    a5, b5, c5, d5, e5, f5, g5, h5,
    a6, b6, c6, d6, e6, f6, g6, h6,
    a7, b7, c7, d7, e7, f7, g7, h7,
    a8, b8, c8, d8, e8, f8, g8, h8
};

struct bitboard {
    uint64_t pawns;
    uint64_t knights;
    uint64_t bishops;
    uint64_t rooks;
    uint64_t queens;
    uint64_t kings;
    uint64_t whites;
    uint64_t moved;
};

struct bitboard* new_board();
void print_board(int * board);
void new_pieces(int * board);
char piece_letter(int piece);
void populate_board( struct bitboard * board );
void rotate_board_180( struct bitboard * board );
int * to_8x8( struct bitboard * board );
int population_count ( uint64_t bitboard );
uint64_t occupied_squares( struct bitboard * board );
uint64_t pawn_moves( struct bitboard * board );
uint64_t shift_n( uint64_t bitlayer );
uint64_t shift_ne( uint64_t bitlayer );
uint64_t shift_nw( uint64_t bitlayer );
uint64_t shift_s( uint64_t bitlayer );
uint64_t shift_se( uint64_t bitlayer );
uint64_t shift_sw( uint64_t bitlayer );
uint64_t shift_e( uint64_t bitlayer );
uint64_t shift_w( uint64_t bitlayer );
uint64_t rotate_180( uint64_t bitlayer );
uint64_t rook_attacks(uint64_t rooks, uint64_t enemies, uint64_t allies);
uint64_t bishop_attacks(uint64_t bishops, uint64_t enemies, uint64_t allies);
uint64_t queen_attacks(uint64_t queens, uint64_t enemies, uint64_t allies);
uint64_t sliding_attack( uint64_t (*slider)(uint64_t), uint64_t attackers, uint64_t enemies, uint64_t allies); 
uint64_t king_attacks(uint64_t kings, uint64_t allies);
uint64_t knight_attacks(uint64_t knights, uint64_t allies);
uint64_t pawn_attacks(uint64_t pawns, uint64_t allies);
