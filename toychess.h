#include<stdint.h>

#define EMPTY 0
#define PAWN 1
#define KNIGHT 2
#define BISHOP 3
#define ROOK 4
#define QUEEN 5
#define KING 6
#define WHITE 8

#define START_POS_FEN "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1"

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

const char * SQUARE_NAMES[] = {
    "a1", "b1", "c1", "d1", "e1", "f1", "g1", "h1",
    "a2", "b2", "c2", "d2", "e2", "f2", "g2", "h2",
    "a3", "b3", "c3", "d3", "e3", "f3", "g3", "h3",
    "a4", "b4", "c4", "d4", "e4", "f4", "g4", "h4",
    "a5", "b5", "c5", "d5", "e5", "f5", "g5", "h5",
    "a6", "b6", "c6", "d6", "e6", "f6", "g6", "h6",
    "a7", "b7", "c7", "d7", "e7", "f7", "g7", "h7",
    "a8", "b8", "c8", "d8", "e8", "f8", "g8", "h8"
};

typedef struct {
    uint64_t pawns;
    uint64_t knights;
    uint64_t bishops;
    uint64_t rooks;
    uint64_t queens;
    uint64_t kings;
    uint64_t whites;
} Bitboard;

typedef struct {
    uint64_t src;
    uint64_t dst;
} Move;

// typedef for where we need a function pointer
typedef uint64_t (*PieceMover)(uint64_t pieces, uint64_t enemies, uint64_t allies);


Bitboard fen_to_board(const char *fen);
void print_board(Bitboard board);
char piece_letter(const int piece);
void rotate_board_180(Bitboard *board);
int population_count(uint64_t bitlayer);
uint64_t occupied_squares(const Bitboard board);
uint64_t shift_n(const uint64_t bitlayer);
uint64_t shift_ne(const uint64_t bitlayer);
uint64_t shift_nw(const uint64_t bitlayer);
uint64_t shift_s(const uint64_t bitlayer);
uint64_t shift_se(const uint64_t bitlayer);
uint64_t shift_sw(const uint64_t bitlayer);
uint64_t shift_e(const uint64_t bitlayer);
uint64_t shift_w(const uint64_t bitlayer);
uint64_t rotate_180( uint64_t bitlayer );
uint64_t pawn_moves(const uint64_t pawns, const uint64_t enemies, const uint64_t allies);
uint64_t rook_attacks(const uint64_t rooks, const uint64_t enemies, const uint64_t allies);
uint64_t bishop_attacks(const uint64_t bishops, const uint64_t enemies, const uint64_t allies);
uint64_t queen_attacks(const uint64_t queens, const uint64_t enemies, const uint64_t allies);
uint64_t king_attacks(const uint64_t kings, const uint64_t enemies, const uint64_t allies);
uint64_t knight_attacks(const uint64_t knights, const uint64_t enemies, const uint64_t allies);
uint64_t pawn_attacks(const uint64_t pawns, const uint64_t allies);
uint64_t sliding_attack( uint64_t (*slider)(uint64_t), uint64_t attackers, const uint64_t enemies, const uint64_t allies);
PieceMover mover_func(const int piece);
uint64_t delete_ls1b(const uint64_t bitlayer, uint64_t *deleted_bit);
int bitscan(const uint64_t b);
int fen_to_piece(const int fen_char);
void add_piece_to_board(Bitboard *board, const int piece, const uint64_t target);
bool in_check(const Bitboard board);
uint64_t standard_attacks(const Bitboard board);
bool can_escape_check(const Bitboard board);
int remove_piece(Bitboard *b, const uint64_t t);
void apply_move(Bitboard *board_ref, const Move move);
int legal_moves(Bitboard board, const uint64_t origin, const uint64_t targets);
int legal_moves_for_piece(const Bitboard board, const int piece);
int legal_moves_for_board(const Bitboard board);
uint64_t squares_with_piece(const Bitboard board, const int piece);
uint64_t src_pieces(const Bitboard board, const uint64_t target, const int piece);
Move parse_algebra(const Bitboard board, const char *algebra);
