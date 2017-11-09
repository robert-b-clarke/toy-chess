# toy-chess

Chess programming hobby project, taken on as I had an urge to re-learn C, and I thought I should learn how to play chess.

Progress ground to a halt several months back, due to a complete lack of spare time. 

## TODO

A lot of things:

* ~~En-passant, castling~~ and pawn promotion
* Algebra support for check, checkmate and special pawn moves
* Human player using algebra interpreter
* Benchmarking (tracepoints, memory profiling etc)
* optimisation (Alpha beta pruning, faster evalutation, reduce rotation of board)
* Experiment with different evaluators, game phases, "openings book" etc
* More diverse set of test cases for comparing algos (could use chess 960 starting positions)
* A "real" UI?

## Build and run the test suite

```bash
make
./testchess
```

That's about it! There is also a board drawing function which can be used to visualize move generation, but there isn't yet a main game play program

## Useful resources

* [Chess programming wiki](https://chessprogramming.wikispaces.com/) - this is a fantastic site, but I've generally tried my own approaches before consulting it
* [Bit twiddling hacks](https://graphics.stanford.edu/~seander/bithacks.html)
