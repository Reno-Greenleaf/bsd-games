snake : snake.o
	g++ -o snake snake.o -lncurses
snake.o: snake.cc pathnames.h
	g++ -I ../../include -c snake.cc
