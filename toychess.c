#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include <ctype.h>
#include "toychess.h"

#define UNUSED(x) (void)(x)

/* const bitmasks for bitboard */
const uint64_t FILE_A = (uint64_t)0x8080808080808080;
const uint64_t FILE_H = (uint64_t)0x0101010101010101;
const uint64_t FILE_AB = (uint64_t)0xC0C0C0C0C0C0C0C0;
const uint64_t FILE_GH = (uint64_t)0x0303030303030303;
const uint64_t RANK_8 = (uint64_t)0x00000000000000FF;
const uint64_t RANK_2 = (uint64_t)0x00FF000000000000;
const uint64_t RANK_1 = (uint64_t)0xFF00000000000000;
const uint64_t LOWEST_SQUARE = (uint64_t)0x0000000000000001;
const uint64_t SQUARE_0 = (uint64_t)0x8000000000000000;
const uint64_t EMPTY_BOARD = (uint64_t)0x0000000000000000;
const uint64_t FULL_BOARD = (uint64_t)0xFFFFFFFFFFFFFFFF;
const uint64_t WHITE_SQUARES = (uint64_t)0x55AA55AA55AA55AA;


void print_board(Bitboard board)
{
    /* print a board to the terminal.
     * as square 0 */
    int file;
    int rank;
    int piece;
    uint64_t target;

    printf("\e[39m\e[49m");
    printf(" ABCDEFGH\n");
    for (rank=0; rank<8; rank++) {
        printf("\e[39m\e[49m");
        printf("%d", 8 - rank);
        for (file=0; file<8; file++) {
            target = SQUARE_0 >> (63 - ((rank * 8) + (7-file)));
            /* choose square background color */
            if (target & WHITE_SQUARES) {
                printf("\e[46m");
            } else {
                printf("\e[45m");
            }
            piece = remove_piece(&board, target);
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


void rotate_board_180(Bitboard *board)
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


int population_count(uint64_t bitlayer)
{
    /* count the set bits */
    int c;
    for (c = 0; bitlayer; bitlayer >>= 1) {
        c += bitlayer & LOWEST_SQUARE;
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


uint64_t king_attacks(uint64_t kings, uint64_t enemies, uint64_t allies)
{
    UNUSED(enemies);
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


Bitboard fen_to_board(char *fen)
{
    /*
     * Parse a string of "Forsyth-Edwards Notation" game state and return a
     * board struct
     */
    int rank = 7; // 0 indexed
    int file = 0;
    // Initialise an empty board
    Bitboard board = {};

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
                &board,
                fen_to_piece(*fen),
                SQUARE_0 >> ((rank * 8) + file)
            );
            file++;
        }
    } while(*fen++ != '\0' && !isspace(*fen));

    // Currently we throw away the remainder of the FEN string
    // Future support can be added for castling, en-passant, game clock etc
    return board;
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


void add_piece_to_board(Bitboard *board, int piece, uint64_t target)
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
    attacks |= king_attacks(board.kings & allies, enemies, allies);
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


int remove_piece(Bitboard *b, uint64_t t) {
    // blank a square and return the piece that resides there
    int white_flag = b->whites & t ? WHITE : 0;
    if(b->kings & t) {
        b->kings ^= t;
        return KING | white_flag;
    } else if(b->queens & t) {
        b->queens ^= t;
        return QUEEN | white_flag;
    } else if(b->rooks & t) {
        b->rooks ^= t;
        return ROOK | white_flag;
    } else if(b->bishops & t) {
        b->bishops ^= t;
        return BISHOP | white_flag;
    } else if(b->knights & t) {
        b->knights ^= t;
        return KNIGHT | white_flag;
    } else if(b->pawns & t) {
        b->pawns ^= t;
        return PAWN | white_flag;
    }
    // square is empty
    return 0;
}


int legal_moves_for_board(Bitboard board) {
    int moves = 0;
    uint64_t allies = occupied_squares(board) & board.whites;
    uint64_t enemies = occupied_squares(board) & ~board.whites;
    // there's only ever one king
    moves += legal_moves(
        board,
        board.kings & allies,
        king_attacks(board.kings & board.whites, enemies, allies),
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
    if(remaining_targets) {
        moves += legal_moves(board, origin, remaining_targets, piece);
    }
    // determine if we're capturing a piece clear it first
    remove_piece(&board, next_target);
    remove_piece(&board, origin);
    add_piece_to_board(&board, piece | WHITE, next_target);
    // assess whether the board is now in check
    rotate_board_180( &board );
    board.whites = ~board.whites;
    if(!in_check(board)) {
        // printf("legal move %c%s\n",
        //     piece_letter(piece),
        //     SQUARE_NAMES[bitscan(next_target)]);
        moves ++;
    }
    return moves;
}


uint64_t src_pieces(Bitboard board, uint64_t target, int piece)
{
    // return the locations of the pieces which can reach a particular square
    uint64_t allies = occupied_squares(board) & board.whites;
    uint64_t enemies = occupied_squares(board) & ~board.whites;
    uint64_t remaining_pieces;
    uint64_t next_piece = EMPTY_BOARD;
    uint64_t srcs = EMPTY_BOARD;
    // TODO - as with legal_moves func there is a lot of cargo cult here
    // a refactor is required
    switch(piece) {
        case PAWN:
            remaining_pieces = board.pawns & allies;
            while(population_count(remaining_pieces) > 0) {
                remaining_pieces = delete_ls1b(remaining_pieces, &next_piece);
                if((pawn_moves(next_piece, enemies, allies)
                    | pawn_attacks(next_piece, enemies)) & target) {
                    srcs |= next_piece;
                }
            }
            return srcs;
        case KNIGHT:
            remaining_pieces = board.knights & allies;
            while(population_count(remaining_pieces) > 0) {
                remaining_pieces = delete_ls1b(remaining_pieces, &next_piece);
                if(knight_attacks(next_piece, allies) & target) {
                    srcs |= next_piece;
                }
            }
            return srcs;
        case ROOK:
            remaining_pieces = board.rooks & allies;
            while(population_count(remaining_pieces) > 0) {
                remaining_pieces = delete_ls1b(remaining_pieces, &next_piece);
                if(rook_attacks(next_piece, enemies, allies)) {
                    srcs |= next_piece;
                }
            }
            return srcs;
        case QUEEN:
            remaining_pieces = board.queens & allies;
            while(population_count(remaining_pieces) > 0) {
                remaining_pieces = delete_ls1b(remaining_pieces, &next_piece);
                if(queen_attacks(next_piece, enemies, allies)) {
                    srcs |= next_piece;
                }
            }
            return srcs;
        case BISHOP:
            remaining_pieces = board.bishops & allies;
            while(population_count(remaining_pieces) > 0) {
                remaining_pieces = delete_ls1b(remaining_pieces, &next_piece);
                if(bishop_attacks(next_piece, enemies, allies)) {
                    srcs |= next_piece;
                }
            }
            return srcs;
        case KING:
            // only one king
            if(king_attacks(board.kings & allies, enemies, allies) & target) {
                return board.kings & allies;
            }
    }
    return srcs;
}

    
Move parse_algebra(Bitboard board, char *algebra)
{
    /*
     * Generously parse a PGN format algebra statement
     * Almost no format validation
     */
    char dst_file = 0;
    char dst_rank = 0;
    int src_piece = PAWN;
    uint64_t src_region = FULL_BOARD;
    uint64_t target_square = EMPTY_BOARD;
    int i;
    Move move = {};
    // loop through statement in reverse
    // take last rank and file as destination
    // use earlier rank or file to disambiguate
    // Assume uppercase char is a piece
    // ignore anything else
    // TODO this is obviously limited!
    for(i = strlen(algebra) - 1; i >= 0; i--) {
        if(isdigit(algebra[i])){
            if(dst_rank) {
                // narrow source pieces by rank
                src_region &= RANK_1 >> ((algebra[i] - 0x31) * 8);
            } else {
                dst_rank = algebra[i];
            }
        } else if(algebra[i] >= 0x61 && algebra[i] <= 0x68) {
            if(dst_file) {
                // narrow source pieces by file
                src_region &= FILE_A >> (algebra[i] - 0x61);
            } else {
                dst_file = algebra[i];
            }
        } else if(isupper(algebra[i])) {
            src_piece = fen_to_piece(algebra[i]) ^ WHITE;
        }
    }
    target_square = (FILE_A >> (dst_file - 0x61)) & (RANK_1 >> ((dst_rank - 0x31) * 8));
    move.src = src_region & src_pieces(board, target_square, src_piece);
    move.dst = target_square;
    // printf("active piece %c going to %s\n", piece_letter(src_piece), SQUARE_NAMES[bitscan(target_square)]);
    return move;
}
