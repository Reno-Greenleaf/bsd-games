snake : snake.o log.o
	g++ -o snake snake.o log.o -lncurses
snake.o : snake.cc pathnames.h snake.h log.h
	g++ -I ../../include -c snake.cc
log.o : log.cc log.h
	g++ -c log.cc
clean :
	rm -f snake *.o log.txt scores.txt