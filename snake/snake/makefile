snake : snake.o log.o screen.o
	g++ -o snake snake.o log.o screen.o -lncurses
snake.o : snake.cc pathnames.h snake.h log.h screen.h
	g++ -I ../../include -c snake.cc
log.o : log.cc log.h
	g++ -c log.cc
screen.o : screen.cc screen.h
	g++ -c screen.cc
clean :
	rm -f snake *.o log.txt scores.txt