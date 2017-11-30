play.o : toychess.o
	gcc -o play play.c
test_chess.o : toychess.o
	gcc -o test_chess test_chess.c
toychess.o : toychess.c toychess.h
	gcc -c toychess.c
clean :
	rm test_chess toychess.o
