#include <stdio.h>
#include <stdint.h>
#include "toychess.c"

/* homebrew unit test stuff */
int assert_board_eq(uint64_t a, uint64_t b, const char *message);
uint64_t sq_map(int location);
void test_king_attacks();
void test_rook_attacks();
void test_knight_attacks();


int main()
{
    test_king_attacks();
    test_rook_attacks();
    test_knight_attacks();
    return 0;
}


void test_king_attacks()
{
    uint64_t kings = sq_map(e4);
    uint64_t attacks = sq_map(d3) | sq_map(d4) | sq_map(d5);
    attacks |= sq_map(f3) | sq_map(f4) | sq_map(f5);
    attacks |= sq_map(e3) | sq_map(e5); 
    assert_board_eq(
        king_attacks(kings, EMPTY_BOARD), 
        attacks, 
        "expected king attacks in open play"
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
        knight_attacks(knights, EMPTY_BOARD), 
        attacks, 
        "expected knight attacks in open play"
    );
}

int assert_board_eq(uint64_t a, uint64_t b, const char *message)
{
    /* compare 2 64 bit bitboard layers and fail noisily if they don't
     * match
     */
    if (a != b) {
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
