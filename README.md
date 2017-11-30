# toy-chess

Chess programming hobby project, taken on as I had an urge to re-learn C, and I thought I should learn how to play chess.

It's a fairly rudimentary implementation which implements (I think) all legal moves, and uses a simple negamax evaluation function for AI, based on Claude Shannon's famous paper on the subject. Board representation is using 64 bit "bitboards".

There are lots of improvements to make, I'll see if I get any time at all to do them!

## Build, play, test

Build

```bash
make
```

Play the computer. Input is via algebraic notation, type `help` to list available moves
```
./play
```

Run the test suite

```bash
./testchess
```

## TODO

A lot of things:

* Benchmarking (tracepoints, memory profiling etc)
* optimisation (Alpha beta pruning, faster evalutation, reduce rotation of board)
* Experiment with different evaluators, game phases, "openings book" etc
* More diverse set of test cases for comparing algos (could use chess 960 starting positions)
* A "real" UI?


## Useful resources

* [Chess programming wiki](https://chessprogramming.wikispaces.com/) - this is a fantastic site, but I've generally tried my own approaches before consulting it
* [Bit twiddling hacks](https://graphics.stanford.edu/~seander/bithacks.html)
* [Claude Shannon - Programming a computer for playing chess (pdf)](http://vision.unipv.it/IA1/aa2009-2010/ProgrammingaComputerforPlayingChess.pdf)
