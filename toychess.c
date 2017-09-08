#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>
#include <ctype.h>
#include "toychess.h"


/* const bitmasks for bitboard */
const uint64_t FILE_A = (uint64_t)0x8080808080808080;
const uint64_t FILE_H = (uint64_t)0x0101010101010101;
const uint64_t FILE_AB = (uint64_t)0xC0C0C0C0C0C0C0C0;
const uint64_t FILE_GH = (uint64_t)0x0303030303030303;
const uint64_t RANK_8 = (uint64_t)0x00000000000000FF;
const uint64_t RANK_2 = (uint64_t)0x00FF000000000000;
const uint64_t LOWEST_SQUARE = (uint64_t)0x0000000000000001;
const uint64_t SQUARE_0 = (uint64_t)0x8000000000000000;
const uint64_t EMPTY_BOARD = (uint64_t)0x0000000000000000;


void print_board(int *board)
{
    /* print a board to the terminal. expects 8x8 format, taking bottom right
     * as square 0 */
    int file;
    int rank;
    int piece;
    printf("\e[39m\e[49m");
    printf(" ABCDEFGH\n");
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
            piece = board[63 - ((rank * 8) + (7-file))];
            if ((piece & WHITE) != 0) {
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
        default:
            return ' ';
    }
}


/*@out@*/ /*@null@*/ struct bitboard* new_board() 
{
    struct bitboard* board = (struct bitboard* )malloc( sizeof(struct bitboard) );
    if (board) {
        /* intitialise everyhing to empty */
        empty_board(board);
    }
    return board;
}


void empty_board(Bitboard * board)
{
    uint64_t empty_row = (uint64_t)0x0000000000000000;
    board->whites = empty_row;
    board->moved = empty_row;
    board->pawns = empty_row;
    board->rooks = empty_row;
    board->knights = empty_row;
    board->bishops = empty_row;
    board->kings = empty_row;
    board->pawns = empty_row;
    board->queens = empty_row;
}


void board_copy(Bitboard src, Bitboard * dst)
{
    dst->pawns = src.pawns;
    dst->rooks = src.rooks;
    dst->knights = src.knights;
    dst->bishops = src.bishops;
    dst->kings = src.kings;
    dst->queens = src.queens;
    dst->whites = src.whites;
    dst->moved = src.moved;
}


void populate_board(Bitboard * board)
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




void rotate_board_180( Bitboard * board )
{
    /*rotate the entire board 180 degrees*/
    board->pawns = rotate_180(board->pawns);
    board->rooks = rotate_180(board->rooks);
    board->knights = rotate_180(board->knights);
    board->bishops = rotate_180(board->bishops);
    board->kings = rotate_180(board->kings);
    board->queens = rotate_180(board->queens);
    board->whites = rotate_180(board->whites);
    board->moved = rotate_180(board->whites);
}


int * to_8x8(Bitboard board)
{
    /* convert bit board into 8x8 hex format for display */
    uint64_t target_piece = SQUARE_0;
    int i;
    int *board_8x8 = malloc(sizeof(int) * 64);
    if (!board_8x8) {
        fprintf(stderr, "Out of memory\n");
        exit(EXIT_FAILURE);
    }
    for (i=0; i < 64; i++) {
        int piece = 0;
        /*target_piece = (uint64_t)0x01 << i;*/
        if (board.pawns & target_piece) {
            piece = PAWN;
        }  else if (board.rooks & target_piece) {
            piece = ROOK;
        }  else if (board.knights & target_piece) {
            piece = KNIGHT;
        }  else if (board.bishops & target_piece) {
            piece = BISHOP;
        }  else if (board.queens & target_piece) {
            piece = QUEEN;
        }  else if (board.kings & target_piece) {
            piece = KING;
        }
        if (board.whites & target_piece) {
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


uint64_t occupied_squares(Bitboard board)
{
    /* any square with a piece on it */
    return board.pawns | board.rooks | board.knights | board.bishops | board.kings | board.queens;
}


uint64_t pawn_moves(uint64_t pawns, uint64_t enemies, uint64_t allies)
{
    uint64_t occupied = enemies | allies;
    uint64_t moves = shift_n(pawns & RANK_2) & ~occupied;
    return shift_n(moves | pawns) & ~occupied;
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


uint64_t pawn_attacks(uint64_t pawns, uint64_t enemies)
{
    /* attack to the ne or nw, only where enemies present */
    uint64_t moves = shift_ne(pawns) | shift_nw(pawns);
    return moves & enemies;
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
    return (bitlayer & ~FILE_H) << 7;
}


uint64_t shift_sw( uint64_t bitlayer )
{
    return (bitlayer & ~FILE_A) << 9;
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
    uint8_t bits_left = 63;
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
    uint8_t position;
    uint8_t file_int = (uint8_t)file - 97;
    rank --;
    position = 8 * (uint8_t)rank + file_int;
    if (position < 64) {
        return SQUARE_0 >> position;
    }
    return EMPTY_BOARD;
}


uint64_t delete_ls1b(uint64_t bitlayer, uint64_t *deleted_bit)
{
    uint64_t without_ls1b = bitlayer & (bitlayer - 1);
    *deleted_bit = bitlayer ^ without_ls1b;
    return without_ls1b;
}

int bitscan ( uint64_t b )
{
    // do binary search, this method with the fixed bitmasks will only work
    // for a single bit, not as a generic leading zero count
    int pos = 32;
    pos += (b & (uint64_t)0xFFFFFFFF00000000) ? -16 : 16;
    pos += (b & (uint64_t)0xFFFF0000FFFF0000) ? -8 : 8;
    pos += (b & (uint64_t)0xFF00FF00FF00FF00) ? -4 : 4;
    pos += (b & (uint64_t)0xF0F0F0F0F0F0F0F0) ? -2 : 2;
    pos += (b & (uint64_t)0XCCCCCCCCCCCCCCCC) ? -1 : 1;
    if (b & (uint64_t)0XAAAAAAAAAAAAAAAA) return pos -1;
    return pos;
}


void fen_to_board(char *fen, Bitboard* board)
{
    /*
     * Parse a string of "Forsyth-Edwards Notation" game state and return a
     * board struct
     */
    int rank = 7; // 0 indexed
    int file = 0;
    // wipe the board to start afresh
    empty_board(board);

    do {
        // if alpha (rnbkqp/RNBKQP) move target west one place
        // if / move east 8 and south 1
        // if numeric 0-8 move west x places
        if (*fen == 0x2F) {
            rank--;
            file = 0;
        } else if (isdigit(*fen)){
            file += (*fen - 0x30);
        } else {
            add_piece_to_board(
                board,
                fen_to_piece(*fen),
                SQUARE_0 >> ((rank * 8) + file)
            );
            file++;
        }
    } while(*fen++ != '\0' && !isspace(*fen));

    // Currently we throw away the remainder of the FEN string
    // Future support can be added for castling, en-passant, game clock etc
}


int fen_to_piece(int fen_char)
{
    // accept a single char and convert to a nibble using piece constants
    int piece;
    switch(tolower(fen_char)) {
        case 112:
            piece = PAWN;
            break;
        case 114:
            piece = ROOK;
            break;
        case 113:
            piece = QUEEN;
            break;
        case 107:
            piece = KING;
            break;
        case 98:
            piece = BISHOP;
            break;
        case 110:
            piece = KNIGHT;
            break;
    }
    // set the white flag bit
    if(isupper(fen_char)) piece |= WHITE;

    return piece;
}


void add_piece_to_board(Bitboard * board, int piece, uint64_t target)
{
    // Add a piece nibble to a board
    if(piece & WHITE) {
        board->whites |= target;
        piece = piece ^ WHITE;
    }
    switch(piece) {
        case PAWN:
            board->pawns |= target;
            break;
        case KNIGHT:
            board->knights |= target;
            break;
        case BISHOP:
            board->bishops |= target;
            break;
        case ROOK:
            board->rooks |= target;
            break;
        case QUEEN:
            board->queens |= target;
            break;
        case KING:
            board->kings |= target;
            break;
    }
}


bool in_check(Bitboard board)
{
    /*
     * return true if white is checking black
     */
    uint64_t attacks = standard_attacks(board);
    if (attacks & (board.kings & ~board.whites)) {
        return true;
    } else {
        return false;
    }
}


uint64_t standard_attacks(Bitboard board)
{
    /*
     * union of all squares white can attack
     * excludes en passant and castling
     */
    uint64_t allies = occupied_squares(board) & board.whites;
    uint64_t enemies = occupied_squares(board) & ~board.whites;

    uint64_t attacks = pawn_attacks(board.pawns & allies, enemies);
    attacks |= rook_attacks(board.rooks & allies, enemies, allies);
    attacks |= queen_attacks(board.queens & allies, enemies, allies);
    attacks |= bishop_attacks(board.bishops & allies, enemies, allies);
    attacks |= knight_attacks(board.knights & allies, allies);
    attacks |= king_attacks(board.kings & allies, allies);
    return attacks;
}


bool can_escape_check(Bitboard board)
{
    /*
     * Test whether we can escape from check, and if we can return true
     * a false return means we're mated
     */
    if(legal_moves_for_board(board) > 0) {
        return true;
    }
    return false;
    
}


void remove_piece(Bitboard * b, uint64_t t) {
    // blank a square
    if(b->kings & t) {
        b->kings ^= t;
    } else if(b->queens & t) {
        b->queens ^= t;
    } else if(b->rooks & t) {
        b->rooks ^= t;
    } else if(b->bishops & t) {
        b->bishops ^= t;
    } else if(b->knights & t) {
        b->knights ^= t;
    } else if(b->pawns & t) {
        b->pawns ^= t;
    }
}


int legal_moves_for_board(Bitboard board) {
    int moves = 0;
    uint64_t allies = occupied_squares(board) & board.whites;
    uint64_t enemies = occupied_squares(board) & ~board.whites;
    // there's only ever one king
    moves += legal_moves(
        board,
        board.kings & allies,
        king_attacks(board.kings & board.whites, allies),
        KING
    );
    
    uint64_t next_piece = EMPTY_BOARD;
    uint64_t remaining_pieces = board.queens & allies;
    // TODO - hideous cargo cult of code for executing available moves
    // use as a basis for testing then refactor to something better
    while(population_count(remaining_pieces) > 0) {
        remaining_pieces = delete_ls1b(remaining_pieces, &next_piece);
        moves += legal_moves(
            board,
            next_piece,
            queen_attacks(next_piece, enemies, allies),
            QUEEN
        );
    }
    remaining_pieces = board.rooks & allies;
    while(population_count(remaining_pieces) > 0) {
        remaining_pieces = delete_ls1b(remaining_pieces, &next_piece);
        moves += legal_moves(
            board,
            next_piece,
            rook_attacks(next_piece, enemies, allies),
            ROOK
        );
    }
    remaining_pieces = board.bishops & allies;
    while(population_count(remaining_pieces) > 0) {
        remaining_pieces = delete_ls1b(remaining_pieces, &next_piece);
        moves += legal_moves(
            board,
            next_piece,
            bishop_attacks(next_piece, enemies, allies),
            BISHOP
        );
    }
    remaining_pieces = board.knights & allies;
    while(population_count(remaining_pieces) > 0) {
        remaining_pieces = delete_ls1b(remaining_pieces, &next_piece);
        moves += legal_moves(
            board,
            next_piece,
            knight_attacks(next_piece, allies),
            KNIGHT
        );
    }
    remaining_pieces = board.pawns & allies;
    while(population_count(remaining_pieces) > 0) {
        remaining_pieces = delete_ls1b(remaining_pieces, &next_piece);
        moves += legal_moves(
            board,
            next_piece,
            pawn_moves(next_piece, enemies, allies) 
                | pawn_attacks(next_piece, enemies),
            PAWN
        );
    }
    return moves;
}



int legal_moves(Bitboard board, uint64_t origin, uint64_t targets, int piece)
{
    int moves = 0;
    uint64_t next_target;
    uint64_t remaining_targets;
    remaining_targets = delete_ls1b(targets, &next_target);
    // determine if we're capturing a piece clear it first
    struct bitboard next_board = {};
    board_copy(board, &next_board);
    remove_piece(&next_board, next_target);
    remove_piece(&next_board, origin);
    add_piece_to_board(&next_board, piece | WHITE, next_target);
    // assess whether the board is now in check
    rotate_board_180( &next_board );
    next_board.whites = ~next_board.whites;
    if(!in_check(next_board)) {
        // printf("legal move %c%s\n",
        //     piece_letter(piece),
        //     SQUARE_NAMES[bitscan(next_target)]);
        moves ++;
    }
    if(remaining_targets) {
        moves += legal_moves(board, origin, remaining_targets, piece);
    }
    return moves;
}
