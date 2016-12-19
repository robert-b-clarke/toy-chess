#include <stdio.h>
#include <stdint.h>
#include "toychess.c"

/* homebrew unit test stuff */
int assert_board_eq(uint64_t a, uint64_t b, const char *message);
int test_king_attacks();
int test_rook_attacks();


int main()
{
    test_king_attacks();
    test_rook_attacks();
    return 0;
}


int test_king_attacks()
{
    uint64_t kings = SQUARE_0 >> e4;
    uint64_t attacks = (SQUARE_0 >> d3) | (SQUARE_0 >> d4) | (SQUARE_0 >> d5);
    attacks |= (SQUARE_0 >> f3) | (SQUARE_0 >> f4) | (SQUARE_0 >> f5);
    attacks |= (SQUARE_0 >> e3) | (SQUARE_0 >> e5); 
    assert_board_eq(
        king_attacks(kings, EMPTY_BOARD), 
        attacks, 
        "expected king attacks in open play"
    );
}


int test_rook_attacks()
{
    uint64_t rooks = SQUARE_0 >> e4;
    uint64_t attacks = (FILE_A >> 4) | (RANK_8 << 32);
    attacks &= ~rooks;
    assert_board_eq(
        rook_attacks(rooks, EMPTY_BOARD, EMPTY_BOARD), 
        attacks, 
        "expected rook attacks in open play"
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
