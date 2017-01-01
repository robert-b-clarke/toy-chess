#include <stdio.h>
#include <stdint.h>
#include "toychess.c"

/* homebrew unit test stuff */
int assert_board_eq(uint64_t a, uint64_t b, const char *message);
uint64_t sq_map(int location);
void test_king_attacks();
void test_rook_attacks();
void test_knight_attacks();
void test_queen_collisions();
void test_rotate_180();
void test_rotate_board();
void test_sq_bit();


int main()
{
    test_king_attacks();
    test_rook_attacks();
    test_knight_attacks();
    test_queen_collisions();
    test_rotate_180();
    test_rotate_board();
    test_sq_bit();
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


void test_rotate_board()
{
    /* rotate an entire board 180 degrees */
    struct bitboard* testboard;
    testboard = new_board();
    populate_board(testboard);
    rotate_board_180(testboard);
    /* white queen should have moved to E8 */
    assert_board_eq(
        testboard->queens & testboard->whites,
        sq_map(e8),
        "White queen has moved to e8"
    );
    /* rotate the board back */
    rotate_board_180(testboard);
    assert_board_eq(
        testboard->queens & testboard->whites,
        sq_map(d1),
        "White queen has moved back to d1"
    );
    free(testboard);
}


void test_sq_bit()
{
    /* Test that conversions of rank and file result in correct
     * single bit being set
     */
    assert_board_eq(
        sq_bit('f', 7),
        sq_map(f7),
        "f7 is f7"
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


uint64_t sq_map(int location)
{
    /* Convenience for returning a bitboard with a single piece on it
     * could be replaced with a lookup table
     */

    return SQUARE_0 >> location;
}
