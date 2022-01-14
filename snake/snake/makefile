snake : snake.o log.o screen.o treasure.o
	g++ -o snake snake.o log.o screen.o treasure.o -lncurses
snake.o : snake.cc pathnames.h snake.h log.h screen.h treasure.h
	g++ -I ../../include -c snake.cc
log.o : log.cc log.h
	g++ -std=c++17 -c log.cc
screen.o : screen.cc screen.h
	g++ -c screen.cc
treasure.o : treasure.cc treasure.h
	g++ -c treasure.cc
clean :
	rm -f snake *.o log.txt scores.txt
	rm -rf ~/.bsd-games/snake