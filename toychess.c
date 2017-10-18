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

const char  *SQUARE_NAMES[] = {
    "a1", "b1", "c1", "d1", "e1", "f1", "g1", "h1",
    "a2", "b2", "c2", "d2", "e2", "f2", "g2", "h2",
    "a3", "b3", "c3", "d3", "e3", "f3", "g3", "h3",
    "a4", "b4", "c4", "d4", "e4", "f4", "g4", "h4",
    "a5", "b5", "c5", "d5", "e5", "f5", "g5", "h5",
    "a6", "b6", "c6", "d6", "e6", "f6", "g6", "h6",
    "a7", "b7", "c7", "d7", "e7", "f7", "g7", "h7",
    "a8", "b8", "c8", "d8", "e8", "f8", "g8", "h8"
};


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
    switch(piece & ~WHITE) {
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


Bitboard enemy_board(Bitboard board)
{
    rotate_board_180(&board);
    board.whites = ~board.whites;
    return board;
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
    moves |= shift_n(moves | pawns) & ~occupied;
    return moves | pawn_attacks(pawns, enemies);
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


uint64_t knight_attacks(uint64_t knights, uint64_t enemies, uint64_t allies)
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
    UNUSED(enemies);
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


PieceMover mover_func(int piece) {
    int uncoloured_piece = piece & ~WHITE;
    switch(uncoloured_piece) {
        case KNIGHT:
            return knight_attacks;
        case BISHOP:
            return bishop_attacks;
        case ROOK:
            return rook_attacks;
        case QUEEN:
            return queen_attacks;
        case KING:
            return king_attacks;
        default:
            return pawn_moves;
    }
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


Bitboard fen_to_board(const char *fen)
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
    attacks |= knight_attacks(board.knights & allies, enemies, allies);
    attacks |= king_attacks(board.kings & allies, enemies, allies);
    return attacks;
}


bool can_escape_check(Bitboard board)
{
    /*
     * Test whether we can escape from check, and if we can return true
     * a false return means we're mated
     */
    // approach is suboptimal as we search all possible moves
    Move *possible_moves = legal_moves_for_board(board);
    if(possible_moves != NULL) {
        return true;
    }
    move_list_delete(&possible_moves);
    return false;
    
}


int piece_at_square(Bitboard b, uint64_t t) {
    int white_flag = b.whites & t ? WHITE : 0;
    if(b.kings & t) {
        return KING | white_flag;
    } else if(b.queens & t) {
        return QUEEN | white_flag;
    } else if(b.rooks & t) {
        return ROOK | white_flag;
    } else if(b.bishops & t) {
        return BISHOP | white_flag;
    } else if(b.knights & t) {
        return KNIGHT | white_flag;
    } else if(b.pawns & t) {
        return PAWN | white_flag;
    }
    // square is empty
    return 0;
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


void legal_moves_for_piece(Move **move_list, Bitboard board, int piece)
{
    // count the legal moves for a given piece on the board
    uint64_t allies = occupied_squares(board) & board.whites;
    uint64_t enemies = occupied_squares(board) & ~board.whites;
    PieceMover piece_mover = mover_func(piece);
    // get the pieces
    uint64_t next_piece = EMPTY_BOARD;
    uint64_t remaining_pieces = squares_with_piece(board, piece) & allies;
    while(population_count(remaining_pieces) > 0) {
        remaining_pieces = delete_ls1b(remaining_pieces, &next_piece);
        legal_moves(
            move_list,
            board,
            next_piece,
            piece_mover(next_piece, enemies, allies)
        );
    }
}


Move *legal_moves_for_board(Bitboard board) {
    // count the moves available for the whole board
    Move *move_list = NULL;
    legal_moves_for_piece(&move_list, board, KING);
    legal_moves_for_piece(&move_list, board, QUEEN);
    legal_moves_for_piece(&move_list, board, ROOK);
    legal_moves_for_piece(&move_list, board, BISHOP);
    legal_moves_for_piece(&move_list, board, KNIGHT);
    legal_moves_for_piece(&move_list, board, PAWN);
    return move_list;
}


void apply_move(Bitboard *board_ref, const Move move) {
    remove_piece(board_ref, move.dst);
    int src_piece = remove_piece(board_ref, move.src);
    add_piece_to_board(board_ref, src_piece, move.dst);
}

void move_list_push(Move **move_list, Move move)
{
    Move *new_move = malloc(sizeof(Move));
    memcpy(new_move, &move, sizeof(Move));
    // insert into head of list
    new_move->next = *move_list;
    *move_list = new_move;
}


int move_list_count(Move *move_list) {
    int i=0;
    Move *next_move = move_list;
    while(next_move != NULL) {
        i++;
        next_move = next_move->next;
    }
    return i;
}


void move_list_delete(Move **move_list) {
    Move *popped;
    while(*move_list != NULL) {
        popped = *move_list;
        *move_list = popped->next;
        free(popped);
    }
}


void legal_moves(Move **move_list, Bitboard board, uint64_t origin, uint64_t targets)
{
    uint64_t next_target;
    uint64_t remaining_targets;
    remaining_targets = delete_ls1b(targets, &next_target);
    if(remaining_targets) {
        legal_moves(move_list, board, origin, remaining_targets);
    }
    // determine if we're capturing a piece clear it first
    Move next_move = {};
    next_move.src = origin;
    next_move.dst = next_target;
    apply_move(&board, next_move);
    // assess whether the board is now in check
    if(!in_check(enemy_board(board))) {
        move_list_push(move_list, next_move);
    }
}


uint64_t squares_with_piece(Bitboard board, int piece)
{
    // return all the squares with a piece, regardless of colour
    switch(piece & ~WHITE) {
        case PAWN:
            return board.pawns;
        case KNIGHT:
            return board.knights;
        case ROOK:
            return board.rooks;
        case QUEEN:
            return board.queens;
        case BISHOP:
            return board.bishops;
        case KING:
            return board.kings;
        default:
            return EMPTY_BOARD;
    }
}


uint64_t src_pieces(Bitboard board, uint64_t target, int piece)
{
    // return the locations of the pieces which can reach a particular square
    uint64_t allies = occupied_squares(board) & board.whites;
    uint64_t enemies = occupied_squares(board) & ~board.whites;
    uint64_t remaining_pieces;
    uint64_t next_piece = EMPTY_BOARD;
    uint64_t srcs = EMPTY_BOARD;
    // find a function which will calculate moves for this piece
    PieceMover piece_mover = mover_func(piece);
    // get the pieces
    remaining_pieces = squares_with_piece(board, piece) & allies;
    // execute the moves for each occurence of the piece
    while(population_count(remaining_pieces) > 0) {
        remaining_pieces = delete_ls1b(remaining_pieces, &next_piece);
        if(piece_mover(next_piece, enemies, allies) & target) {
            srcs |= next_piece;
        }
    }
    return srcs;
}

char *algebra_for_move(Bitboard board, Move move)
{
    int piece = piece_at_square(board, move.src);
    int file_neighbours; 
    int rank_neighbours; 
    char src_file = 0;
    char src_rank = 0;
    // work out if we're ambiguous
    uint64_t srcs = src_pieces(board, move.dst, move.src);
    if(population_count(srcs) > 1) {
        // disambig on file or on src
        file_neighbours = population_count(
            sliding_attack(shift_n, move.src, EMPTY_BOARD, EMPTY_BOARD)
            | sliding_attack(shift_s, move.src, EMPTY_BOARD, EMPTY_BOARD)
        );
        rank_neighbours = population_count(
            sliding_attack(shift_e, move.src, EMPTY_BOARD, EMPTY_BOARD)
            | sliding_attack(shift_w, move.src, EMPTY_BOARD, EMPTY_BOARD)
        );
        printf("ambiguous with file neighbours %d and rank neighbours %d", file_neighbours, rank_neighbours);
        int loc = bitscan(move.dst);
        // file loc % 8;
        if(file_neighbours == 0){
            src_rank = 0x31 + abs(loc / 8);
        }
        // rank floor(loc / 8)
        
    }
    // build up string
    char algebra[10];
    char *c = algebra;
    char letter = piece_letter(piece);
    bool captures = (piece_at_square(board, move.dst)) ? true  : false;
    if(letter != 112) {
        *c = letter;
        c++;
    }
    if(src_file) {
        *c = src_file;
        c++;
    }
    if(src_rank) {
        *c = src_rank;
        c++;
    }
    if(captures) {
        *c = 'x';
        c++;
    }
    // copy target square onto the end of the string
    strcpy(c, SQUARE_NAMES[bitscan(move.dst)]);
    char *result = malloc(strlen(algebra));
    strcpy(result, algebra);
    return result;
}


Move parse_algebra(Bitboard board, const char *algebra)
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
