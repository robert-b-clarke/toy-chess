#include <stdio.h>
#include <stdint.h>
#include "toychess.c"

/* homebrew unit test stuff */
int assert_board_eq(uint64_t a, uint64_t b, const char *message);
int assert_true(bool condition, const char *message);
uint64_t sq_map(int location);
void test_king_attacks();
void test_rook_attacks();
void test_knight_attacks();
void test_pawn_attacks();
void test_pawn_moves();
void test_queen_collisions();
void test_rotate_180();
void test_upside_down();
void test_rotate_board();
void test_delete_ls1b();
void test_bitscan();
void test_fen_to_board();
void test_in_check();
void test_escape_check();
void test_src_pieces();
void test_parse_algebra();
void test_move_count();
void test_castling_move_generation();
void test_enpassant();
void test_pawn_promotion();


int main()
{
    test_king_attacks();
    test_rook_attacks();
    test_knight_attacks();
    test_queen_collisions();
    test_pawn_moves();
    test_pawn_attacks();
    test_rotate_180();
    test_upside_down();
    test_rotate_board();
    test_delete_ls1b();
    test_bitscan();
    test_fen_to_board();
    test_in_check();
    test_escape_check();
    test_src_pieces();
    test_parse_algebra();
    test_move_count();
    test_castling_move_generation();
    test_enpassant();
    test_pawn_promotion();
    match_player(negamax_mover, human_mover);
    return 0;
}


void test_king_attacks()
{
    uint64_t kings = sq_map(e4);
    uint64_t attacks = sq_map(d3) | sq_map(d4) | sq_map(d5);
    attacks |= sq_map(f3) | sq_map(f4) | sq_map(f5);
    attacks |= sq_map(e3) | sq_map(e5);
    assert_board_eq(
        king_attacks(kings, EMPTY_BOARD, EMPTY_BOARD),
        attacks, 
        "expected king attacks in open play"
    );
    // test king in bottom right corner
    assert_board_eq(
        king_attacks(sq_map(h1), EMPTY_BOARD, EMPTY_BOARD),
        sq_map(g1) | sq_map(g2) | sq_map(h2),
        "expected king attacks when trapped in bottom right corner"
    );
    // test king in bottom left corner
    assert_board_eq(
        king_attacks(sq_map(a1), EMPTY_BOARD, EMPTY_BOARD),
        sq_map(b1) | sq_map(b2) | sq_map(a2),
        "expected king attacks when trapped in bottom left corner"
    );
    // test king in top left corner
    assert_board_eq(
        king_attacks(sq_map(a8), EMPTY_BOARD, EMPTY_BOARD),
        sq_map(b8) | sq_map(b7) | sq_map(a7),
        "expected king attacks when trapped in top left corner"
    );
    // test king in top right corner
    assert_board_eq(
        king_attacks(sq_map(h8), EMPTY_BOARD, EMPTY_BOARD),
        sq_map(g8) | sq_map(g7) | sq_map(h7),
        "expected king attacks when trapped in top right corner"
    );
}


void test_pawn_attacks()
{
    uint64_t pawns = sq_map(b2) | sq_map(c2);
    uint64_t opponents = sq_map(c3) | sq_map(e3);
    assert_board_eq(
        pawn_attacks(pawns, opponents),
        sq_map(c3),
        "pawns can attack diagonally to enemy squaress"
    );
}


void test_pawn_moves()
{
    uint64_t fresh_pawns = sq_map(a2);
    uint64_t moved_pawns = sq_map(b3);
    assert_board_eq(
        pawn_moves(
            fresh_pawns | moved_pawns,
            EMPTY_BOARD,
            EMPTY_BOARD
        ),
        sq_map(a3) | sq_map(a4) | sq_map(b4),
        "Without obstacles pawns can move forward 1 or 2 spaces"
    );
    // test a collision
    assert_board_eq(
        pawn_moves(sq_map(a2), sq_map(a4), EMPTY_BOARD),
        sq_map(a3),
        "Advance of pawn impeded by allied piece"
    );
}


void test_rook_attacks()
{
    uint64_t rooks = sq_map(e4);
    uint64_t attacks = (FILE_A >> 4) | (RANK_8 << 32);
    attacks &= ~rooks;
    assert_board_eq(
        rook_attacks(rooks, EMPTY_BOARD, EMPTY_BOARD), 
        attacks, 
        "expected rook attacks in open play"
    );
}


void test_knight_attacks()
{
    uint64_t knights = sq_map(e4);
    uint64_t attacks = sq_map(f6) | sq_map(g5) | sq_map(g3) | sq_map(f2);
    attacks |= sq_map(d2) | sq_map(c3) | sq_map(c5) | sq_map(d6);
    assert_board_eq(
        knight_attacks(knights, EMPTY_BOARD, EMPTY_BOARD), 
        attacks, 
        "expected knight attacks in open play"
    );
}


void test_queen_collisions()
{
    uint64_t queens = sq_map(b2);
    uint64_t allies = sq_map(a3) | sq_map(b3) | sq_map(c3);
    uint64_t enemies = sq_map(d1) | sq_map(d2) | sq_map(d3);
    /* queen should be able to attack up to borders with allies and overlap
     * with enemies
     */
    uint64_t attacks = sq_map(a1) | sq_map(b1) | sq_map(c1);
    attacks |= sq_map(a2) | sq_map(c2) | sq_map(d2);
    assert_board_eq(
        queen_attacks(queens, enemies, allies),
        attacks,
        "expected queen attacks when boxed in"
    );
}


void test_rotate_180()
{
    assert_board_eq(
        rotate_180(sq_map(a8)),
        sq_map(h1),
        "A8 has moved to H1"
    );
}

void test_upside_down()
{
    Bitboard board = {};
    board.rooks = upside_down(sq_map(a8));
    print_board(board);
    assert_board_eq(
        upside_down(sq_map(a8)),
        sq_map(a1),
        "A8 has moved to A1"
    );
}



void test_rotate_board()
{
    /* rotate an entire board 180 degrees */
    Bitboard testboard = fen_to_board(START_POS_FEN);
    rotate_board_180(&testboard);
    /* white queen should have moved to E8 */
    assert_board_eq(
        testboard.queens & testboard.whites,
        sq_map(e8),
        "White queen has moved to e8"
    );
    /* rotate the board back */
    rotate_board_180(&testboard);
    assert_board_eq(
        testboard.queens & testboard.whites,
        sq_map(d1),
        "White queen has moved back to d1"
    );
}


void test_delete_ls1b()
{
    /* Test we can extract bits sequentially */
    uint64_t extracted_bit;
    uint64_t board = sq_map(f8) | sq_map(e3);
    board = delete_ls1b(board, &extracted_bit);
    assert_board_eq(
        board,
        sq_map(e3),
        "f8 has been removed from board"
    );
    assert_board_eq(
        extracted_bit,
        sq_map(f8),
        "f8 has been allocated to extracted bit"
    );
    // extract another bit
    board = delete_ls1b(board, &extracted_bit);
    assert_board_eq(
        board,
        EMPTY_BOARD,
        "Board is now empty"
    );
    assert_board_eq(
        extracted_bit,
        sq_map(e3),
        "e3 has been allocated to extracted bit"
    );
}


void test_bitscan()
{
    uint64_t board = 0x8000000000000000;
    int i;
    for(i=0; i<64; i++) {
        int bitscan_result = bitscan(board >> i);
        if(i != bitscan_result) {
            fprintf(
                stderr,
                "FAIL: bitscan %d does not match offset %d\n",
                bitscan_result, i
            );
            exit(1);
        }
    }
}


void test_fen_to_board()
{
    /*
     * Test that a Forsyth-Edwards Notation game serialization can be unpacked
     * into a board struct
     */
    Bitboard testboard;
    testboard = fen_to_board(START_POS_FEN);
    assert_board_eq(
        testboard.pawns,
        (uint64_t)0x00FF00000000FF00,
        "pawns initialised correctly"
    );
    assert_board_eq(
        testboard.rooks,
        (uint64_t)0x8100000000000081,
        "rooks initialised correctly"
    );
    assert_board_eq(
        testboard.knights,
        (uint64_t)0x4200000000000042,
        "knights initialised correctly"
    );
    assert_board_eq(
        testboard.bishops,
        (uint64_t)0x2400000000000024,
        "bishops initialised correctly"
    );
    assert_board_eq(
        testboard.kings,
        (uint64_t)0x0800000000000008,
        "kings initialised correctly"
    );
    assert_board_eq(
        testboard.queens,
        (uint64_t)0x1000000000000010,
        "queens initialised correctly"
    );
    assert_board_eq(
        testboard.whites,
        (uint64_t)0xFFFF000000000000,
        "bishops initialised correctly"
    );
    assert_true(
        testboard.castle_wks,
        "White kingside castling is allowed"
    );
    assert_true(
        testboard.castle_wqs,
        "White queenside castling is allowed"
    );
    assert_true(
        testboard.castle_bks,
        "Black kingside castling is allowed"
    );
    assert_true(
        testboard.castle_bqs,
        "Black queenside castling is allowed"
    );
    // print out the board for good measure
    print_board(testboard);
}


void test_in_check()
{
    char not_in_check[] = "8/8/8/8/2k5/8/8/QK6";
    char check[] = "8/8/8/8/3k4/8/8/QK6";
    // Test a board that is not in check
    Bitboard testboard;
    testboard = fen_to_board(not_in_check);
    assert_true(
        !in_check(testboard),
        "Correctly detect board is not in check"
    );
    // Test a board that is in check and fail if we don't
    testboard = fen_to_board(check);
    assert_true(
        in_check(testboard),
        "Correctly detect board is in check"
    );
}

void test_escape_check()
{
    char king_flees[] = "8/8/8/8/3K4/8/8/qk6";
    char king_trapped[] = "8/8/1k6/4b3/4b3/8/P7/K7";
    char knight_interposes[] = "8/8/1pk5/8/3B4/N7/PP5Q/K4r2";
    char pawn_captures[] = "8/8/2k5/8/p3b3/1np5/P7/K7";
    char pawn_pinned[] = "8/8/2k5/8/r3b3/1np5/P7/K7";

    Bitboard testboard;
    testboard = fen_to_board(king_flees);
    assert_true(
        can_escape_check(testboard),
        "correctly determine our king can flee check"
    );
    testboard = fen_to_board(king_trapped);
    print_board(testboard);
    assert_true(
        !can_escape_check(testboard),
        "correctly determine white is mated"
    );
    testboard = fen_to_board(knight_interposes);
    assert_true(
        can_escape_check(testboard),
        "correctly determine we can interpose the white Knight"
    );
    testboard = fen_to_board(pawn_captures);
    assert_true(
        can_escape_check(testboard),
        "correctly determine we can interpose the white Knight"
    );
    testboard = fen_to_board(pawn_pinned);
    assert_true(
        !can_escape_check(testboard),
        "We can't escape check as the white pawn is \"pinned\" by rook"
    );
}

int assert_board_eq(uint64_t a, uint64_t b, const char *message)
{
    /* compare 2 64 bit bitboard layers and fail noisily if they don't
     * match
     */
    if (a != b) {
        fprintf(stderr, "FAIL: %s\n", message);
        printf("%d differences between boards\n", population_count(b ^ a));
        exit(1);
    }
    return 1;
}


int assert_true(bool condition, const char *message)
{
    /*
     * fail noisily if condition not true
     */
    if (!condition) {
        fprintf(stderr, "FAIL: %s\n", message);
        exit(1);
    }
    return 1;
}


uint64_t sq_map(int location)
{
    /* Convenience for returning a bitboard with a single piece on it
     * could be replaced with a lookup table
     */

    return SQUARE_0 >> location;
}


void test_src_pieces()
{
    // https://chess.stackexchange.com/questions/1817/how-are-pgn-ambiguities-handled
    char weird_board[] = "1R4QQ/R1R4Q/8/6pP/5P1P/8/NK1k4/1N1N4";
    Bitboard testboard;
    testboard = fen_to_board(weird_board);
    print_board(testboard);
    // test ambiguous pawn moves
    uint64_t srcs = src_pieces(testboard, sq_map(g5), PAWN);
    assert_board_eq(
        srcs,
        sq_map(f4) | sq_map(h4),
        "2 pawns can capture g5"
    );
    // test ambiguous rook moves
    srcs = src_pieces(testboard, sq_map(b7), ROOK);
    assert_board_eq(
        srcs,
        sq_map(a7) | sq_map(b8) | sq_map(c7),
        "3 rooks can go to b7"
    );
    // test ambiguous knight moves
    srcs = src_pieces(testboard, sq_map(c3), KNIGHT);
    assert_board_eq(
        srcs,
        sq_map(a2) | sq_map(b1) | sq_map(d1),
        "3 knights can go to c3"
    );
    // test ambiguous queen moves
    srcs = src_pieces(testboard, sq_map(g7), QUEEN);
    assert_board_eq(
        srcs,
        sq_map(g8) | sq_map(h8) | sq_map(h7),
        "3 knights can go to c3"
    );
    // test king move
    srcs = src_pieces(testboard, sq_map(c3), KING);
    assert_board_eq(
        srcs,
        sq_map(b2),
        "A single king can move to c3"
    );
    // test bishop move with no availablie bishops
    srcs = src_pieces(testboard, sq_map(c3), BISHOP);
    assert_board_eq(
        srcs,
        EMPTY_BOARD,
        "No bishops available"
    );
    // test src_pieces for black move
    testboard.black_move = true;
    srcs = src_pieces(testboard, sq_map(e3), KING);
    assert_board_eq(
        srcs,
        sq_map(d2),
        "Black king is available"
    );

}
    
void test_parse_algebra()
{
    char weird_board[] = "1R4QQ/R1R4Q/8/6pP/5P1P/8/NK1k4/1N1N4";
    Bitboard testboard;
    testboard = fen_to_board(weird_board);
    Move move;
    move = parse_algebra(testboard, "Nc3");
    assert_board_eq(
        move.dst,
        EMPTY_BOARD,
        "Ambiguous move specifies no destination"
    );
    assert_board_eq(
        move.src,
        EMPTY_BOARD,
        "Ambiguous move specifies no source"
    );

    move = parse_algebra(testboard, "Nbc3");
    assert_board_eq(
        move.src,
        sq_map(b1),
        "Move disambiguated by file"
    );
    // test  disambiguation by rank
    char ambig_knights_board[] = "1R4QQ/R1R4Q/8/6pP/NP1P/8/NK1k4/8";
    testboard = fen_to_board(ambig_knights_board);
    move = parse_algebra(testboard, "N4c3");
    assert_board_eq(
        move.src,
        sq_map(a4),
        "Move disambiguated by rank"
    );
    char black_response[] =
        "rnbqkbnr/pppppppp/8/8/3P4/8/PPP1PPPP/RNBQKBNR b KQkq - 1 1";
    testboard = fen_to_board(black_response);
    move = parse_algebra(testboard, "Nc6");
    assert_board_eq(
        move.src,
        sq_map(b8),
        "Move disambiguated by rank"
    );
}


void test_move_count()
{
    Bitboard testboard = fen_to_board(START_POS_FEN);
    Move *move_list = legal_moves_for_board(testboard);
    assert_true(
        move_list_count(move_list) == 20,
        "20 opening moves are available"
    );
    move_list_delete(&move_list);
}


void test_castling_move_generation()
{
    char *algebra;
    // unrealistic position which would allow for castling
    Bitboard testboard = fen_to_board(
        "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQK2R w KQkq - 0 1"
    );
    Move *move_list = NULL;
    legal_moves_castling(&move_list, testboard);
    assert_true(
        move_list_count(move_list) == 1,
        "1 castling move available for white kingside"
    );
    // apply the move and check the results are as expected
    apply_move(&testboard, *move_list);
    assert_board_eq(
        testboard.rooks,
        sq_map(a8) | sq_map(h8) | sq_map(a1) | sq_map(f1),
        "Kingside rook has moved"
    );
    // check algebra looks realistic
    algebra = algebra_for_move(testboard, *move_list);
    assert_true(
        strcmp(algebra, "0-0") == 0,
        "Correct algebra for castling"
    );
    free(algebra);
    move_list_delete(&move_list);
    // start again with white queenside
    testboard = fen_to_board(
        "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/R3KBNR w KQkq - 0 1"
    );
    move_list = NULL;
    legal_moves_castling(&move_list, testboard);
    assert_true(
        move_list_count(move_list) == 1,
        "1 castling move available for white queenside"
    );
    apply_move(&testboard, *move_list);
    assert_board_eq(
        testboard.rooks,
        sq_map(a8) | sq_map(h8) | sq_map(d1) | sq_map(h1),
        "Kingside rook has moved"
    );
    algebra = algebra_for_move(testboard, *move_list);
    assert_true(
        strcmp(algebra, "0-0-0") == 0,
        "Correct algebra for castling"
    );
    free(algebra);
    move_list_delete(&move_list);
    // Repeat for black kingside
    testboard = fen_to_board(
        "rnbqk2r/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR b KQkq - 0 1"
    );
    move_list = NULL;
    legal_moves_castling(&move_list, testboard);
    assert_true(
        move_list_count(move_list) == 1,
        "1 castling move available for black kingside"
    );
    apply_move(&testboard, *move_list);
    move_list_delete(&move_list);
    // Repeat for black queenside
    testboard = fen_to_board(
        "r3kbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR b KQkq - 0 1"
    );
    move_list = NULL;
    print_board(testboard);
    legal_moves_castling(&move_list, testboard);
    assert_true(
        move_list_count(move_list) == 1,
        "1 castling move available for black queenside"
    );
    apply_move(&testboard, *move_list);
    print_board(testboard);
    move_list_delete(&move_list);
}


void test_enpassant() {
    Bitboard testboard = fen_to_board(
        "rnbqkbnr/1p1ppppp/p7/2pP4/8/8/PPP1PPPP/RNBQKBNR w KQkq c6 0 2"
    );
    assert_board_eq(
        testboard.enpassant,
        sq_map(c6),
        "Correctly parsed en-passant square"
    );
    // check that move generation includes enpassant
    Move *move_list = legal_moves_for_board(testboard);
    Move *move_ptr = move_list;
    while(move_ptr != NULL) {
        if(move_ptr->special & ENPASSANT)
            break;
        move_ptr = move_ptr->next;
    }
    assert_board_eq(
        move_ptr->dst,
        sq_map(c6),
        "generated move targets C6"
    );
    assert_board_eq(
        move_ptr->src,
        sq_map(d5),
        "generated move originates from D5"
    );
    // test en-passant algebra looks OK
    char *algebra = algebra_for_move(testboard, *move_ptr);
    assert_true(
        strcmp(algebra, "dxc6") == 0,
        "En-Passant algebra correct"
    );
    free(algebra);
    // actually apply the move
    apply_move(&testboard, *move_ptr);
    // test we've blanked the opponents pawn
    assert_board_eq(
        testboard.pawns & sq_map(d6),
        EMPTY_BOARD,
        "Black pawn removed in enpassant capture"
    );
    // test that enpassant target is cleared
    assert_true(
        testboard.enpassant==EMPTY_BOARD,
        "En-passant target cleared"
    );
    move_list_delete(&move_list);
    // test that enpassant square is actually set when it should be
    Move double_push = {};
    double_push.src = sq_map(g7);
    double_push.dst = sq_map(g5);
    apply_move(&testboard, double_push);
    assert_board_eq(
        testboard.enpassant,
        sq_map(g6),
        "Correctly set en-passant square"
    );
}


void test_pawn_promotion()
{
    Bitboard testboard = fen_to_board("1r6/P6k/8/8/8/8/8/7K");
    Move *move_list = NULL;
    legal_moves_for_pawns(&move_list, testboard);
    assert_true(
        move_list_count(move_list)==10,
        "10 moves available"
    );
    move_list_delete(&move_list);
    char *algebra;
    Move queen_promote = {};
    queen_promote.src = sq_map(a7);
    queen_promote.dst = sq_map(b8);
    queen_promote.special = PROMOTE_QUEEN;
    algebra = algebra_for_move(testboard, queen_promote);
    assert_true(
        strcmp(algebra, "xb8=Q")==0,
        "Correct algebra for capture promoted to QUEEN"
    );
    free(algebra);
    apply_move(&testboard, queen_promote);
    assert_true(
        piece_at_square(testboard, sq_map(b8)) == WHITE | QUEEN,
        "pawn promoted to white queen"
    );
}
