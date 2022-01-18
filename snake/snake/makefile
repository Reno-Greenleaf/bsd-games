snake : snake.o log.o screen.o treasure.o room.o finish.o me.o monster.o
	g++ -o snake snake.o log.o screen.o treasure.o room.o finish.o me.o monster.o -lncurses

snake.o : snake.cc pathnames.h snake.h log.h screen.h treasure.h room.h finish.h me.h monster.h interfaces.h
	g++ -I ../../include -c snake.cc

log.o : log.cc log.h interfaces.h
	g++ -std=c++17 -c log.cc

screen.o : screen.cc screen.h interfaces.h
	g++ -c screen.cc

treasure.o : treasure.cc treasure.h interfaces.h
	g++ -c treasure.cc

room.o : room.cc room.h interfaces.h
	g++ -c room.cc

finish.o : finish.cc finish.h interfaces.h
	g++ -c finish.cc

me.o : me.cc me.h interfaces.h
	g++ -c me.cc

monster.o : monster.cc monster.h interfaces.h
	g++ -c monster.cc

clean :
	rm -f snake *.o scores.txt
	rm -rf ~/.bsd-games/snake