/*	$NetBSD: snake.c,v 1.20 2004/02/08 00:33:31 jsm Exp $	*/

/*
 * Copyright (c) 1980, 1993
 *	The Regents of the University of California.  All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the University nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

#include <sys/cdefs.h>
#ifndef lint
__COPYRIGHT("@(#) Copyright (c) 1980, 1993\n\
	The Regents of the University of California.  All rights reserved.\n");
#endif				/* not lint */

#ifndef lint
#if 0
static char sccsid[] = "@(#)snake.c	8.2 (Berkeley) 1/7/94";
#else
__RCSID("$NetBSD: snake.c,v 1.20 2004/02/08 00:33:31 jsm Exp $");
#endif
#endif				/* not lint */

/*
 * snake - crt hack game.
 *
 * You move around the screen with arrow keys trying to pick up money
 * without getting eaten by the snake.  hjkl work as in vi in place of
 * arrow keys.  You can leave at the exit any time.
 *
 * compile as follows:
 *	cc -O snake.c move.c -o snake -lm -ltermlib
 */

#include <sys/param.h>

#include <curses.h>
#include <fcntl.h>
#include <pwd.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <sys/types.h>
#include <err.h>
#include <math.h>
#include <signal.h>
#include <stdio.h>
#include <string.h>
#include <termios.h>

#include "pathnames.h"
#include "snake.h"
#include "log.h"
#include "screen.h"
#include "treasure.h"
#include "finish.h"
#include "room.h"
#include "me.h"
#include "monster.h"

#define cashvalue chunk*(loot-penalty)/25
#define	same(s1, s2) ((s1)->line == (s2)->line && (s1)->col == (s2)->col)

#define PENALTY  10		/* % penalty for invoking spacewarp	 */

#define EOT	'\004'
#define LF	'\n'
#define DEL	'\177'

#define delay(t)	usleep(t * 50000);

struct point you;
struct point money;
struct point finish;

namespace snake {
	Treasure money;
	Log log;
	Screen screen;
	Room room;
	Finish finish;
	Me you;
	Monster monster;
	std::array<struct point, 6> snake;
}

int loot, penalty;
int moves;
int fast = 1;

int rawscores;
FILE *logfile;

int	lcnt, ccnt;	/* user's idea of screen size */

namespace settings {
	int horizontalLimit;
	int verticalLimit;
}

int	chunk; /* amount of money given at a time */

int main(int argc, char **argv)
{
	int     ch, i;
	time_t tv;

	std::vector<snake::IBody*> obstacles {&snake::you, &snake::room, &snake::finish, &snake::monster, &snake::money};
	snake::log.load();

	/* Open score files then revoke setgid privileges */
	rawscores = open(_PATH_RAWSCORES, O_RDWR|O_CREAT, 0664);

	if (rawscores < 0) {
		warn("open %s", _PATH_RAWSCORES);
		sleep(2);
	} else if (rawscores < 3)
		exit(1);

	setregid(getgid(), getgid());

	(void) time(&tv);

	while ((ch = getopt(argc, argv, "l:w:t")) != -1)
		switch ((char) ch) {
#ifdef DEBUG
		case 'd':
			tv = atol(optarg);
			break;
#endif
		case 'w':	/* width */
			settings::horizontalLimit = ccnt = atoi(optarg);
			break;
		case 'l':	/* length */
			settings::verticalLimit = lcnt = atoi(optarg);
			break;
		case 't':
			fast = 0;
			break;
		case '?':
		default:
#ifdef DEBUG
			fputs("usage: snake [-d seed] [-w width] [-l length] [-t]\n", stderr);
#else
			fputs("usage: snake [-w width] [-l length] [-t]\n", stderr);
#endif
			exit(1);
		}

	srandom((int) tv);

	penalty = loot = 0;

	if (!snake::screen.load()) {
		snake::log.write("error: the screen wasn't prepared.");
		exit(1);
	}

	cbreak();
	noecho();
#ifdef KEY_LEFT
	keypad(stdscr, TRUE);
#endif
	if (!lcnt || lcnt > LINES - 2)
		settings::verticalLimit = lcnt = LINES - 2;

	if (!ccnt || ccnt > COLS - 2)
		settings::horizontalLimit = ccnt = COLS - 2;

	i = fmin(lcnt, ccnt);

	if (i < 4) {
		endwin();
		errx(1, "screen too small for a fair game.");
	}

	snake::room.warp(obstacles);

	/*
	 * chunk is the amount of money the user gets for each $.
	 * The formula below tries to be fair for various screen sizes.
	 * We only pay attention to the smaller of the 2 edges, since
	 * that seems to be the bottleneck.
	 * This formula is a hyperbola which includes the following points:
	 *	(24, $25)	(original scoring algorithm)
	 *	(12, $40)	(experimentally derived by the "feel")
	 *	(48, $15)	(a guess)
	 * This will give a 4x4 screen $99/shot.  We don't allow anything
	 * smaller than 4x4 because there is a 3x3 game where you can win
	 * an infinite amount of money.
	 */
	if (i < 12)
		i = 12;		/* otherwise it isn't fair */
	/*
	 * Compensate for border.  This really changes the game since
	 * the screen is two squares smaller but we want the default
	 * to be $25, and the high scores on small screens were a bit
	 * much anyway.
	 */
	i += 2;
	chunk = (675.0 / (i + 6)) + 2.5;	/* min screen edge */

	signal(SIGINT, stop);

	you = snake::you.warp(obstacles);
	finish = snake::finish.warp(obstacles);
	money = snake::money.warp(obstacles);
	snake::monster.warp(obstacles); snake::snake = snake::monster.warp();

	snake::monster.avoid(&snake::money);
	snake::monster.avoid(&snake::room);
	snake::monster.avoid(&snake::finish);

	setup();
	mainloop(obstacles);
	/* NOTREACHED */
	return (0);
}

struct point* point(struct point* ps, int x, int y)
{
	ps->col = x;
	ps->line = y;
	return (ps);
}

/* Main command loop */
void mainloop(std::vector<snake::IBody*> obstacles)
{
	int k;
	int repeat = 1;
	int	lastc = 0;

	for (;;) {
		int c;

		/* Highlight you, not left & above */
		snake::you.display(snake::screen);
		refresh();

		if (((c = getch()) <= '9') && (c >= '0')) {
			repeat = c - '0';

			while (((c = getch()) <= '9') && (c >= '0'))
				repeat = 10 * repeat + (c - '0');

		} else {
			if (c != '.')
				repeat = 1;
		}

		if (c == '.') {
			c = lastc;
		}

		if (!fast)
			flushi();

		lastc = c;

		switch (c) {
			case CTRL('z'):
				suspend();
				continue;
			case EOT:
			case 'x':
			case 0177:	/* del or end of file */
				endwin();
				length(moves);
				snake::log.write("quit", cashvalue, ccnt, lcnt);
				exit(0);
			case CTRL('l'):
				setup();
				winnings(cashvalue);
				continue;
			case 'p':
			case 'd':
				snap();
				continue;
			case 'w':
				spacewarp(0, obstacles);
				continue;
			case 'A':
				repeat = you.col;
				c = 'h';
				break;
			case 'H':
			case 'S':
				repeat = you.col - money.col;
				c = 'h';
				break;
			case 'T':
				repeat = you.line;
				c = 'k';
				break;
			case 'K':
			case 'E':
				repeat = you.line - money.line;
				c = 'k';
				break;
			case 'P':
				repeat = ccnt - 1 - you.col;
				c = 'l';
				break;
			case 'L':
			case 'F':
				repeat = money.col - you.col;
				c = 'l';
				break;
			case 'B':
				repeat = lcnt - 1 - you.line;
				c = 'j';
				break;
			case 'J':
			case 'C':
				repeat = money.line - you.line;
				c = 'j';
				break;
		}

		for (k = 1; k <= repeat; k++) {
			moves++;
		
			switch (c) {
				case 's':
				case 'h':
#ifdef KEY_LEFT
				case KEY_LEFT:
#endif
				case '\b':
					if (!snake::room.occupies(you.col-1, you.line)) {

						if ((fast) || (k == 1))
							snake::screen.print(' ', you.col, you.line, snake::BLACK);

						you.col--;
						snake::you.move(-1, 0);

						if ((fast) || (k == repeat) || (you.col == 0))
							snake::you.display(snake::screen);
					}
					break;
				case 'f':
				case 'l':
#ifdef KEY_RIGHT
				case KEY_RIGHT:
#endif
				case ' ':
					if (!snake::room.occupies(you.col+1, you.line)) {

						if ((fast) || (k == 1))
							snake::screen.print(' ', you.col, you.line, snake::BLACK);

						you.col++;
						snake::you.move(1, 0);

						if ((fast) || (k == repeat) || (you.col == ccnt - 1))
							snake::you.display(snake::screen);
					}
					break;
				case CTRL('p'):
				case 'e':
				case 'k':
#ifdef KEY_UP
				case KEY_UP:
#endif
				case 'i':
					if (!snake::room.occupies(you.col, you.line-1)) {

						if ((fast) || (k == 1))
							snake::screen.print(' ', you.col, you.line, snake::BLACK);

						you.line--;
						snake::you.move(0, -1);

						if ((fast) || (k == repeat) || (you.line == 0))
							snake::you.display(snake::screen);
					}

					break;
				case CTRL('n'):
				case 'c':
				case 'j':
#ifdef KEY_DOWN
				case KEY_DOWN:
#endif
				case LF:
				case 'm':
					if (!snake::room.occupies(you.col, you.line+1)) {

						if ((fast) || (k == 1))
							snake::screen.print(' ', you.col, you.line, snake::BLACK);

						you.line++;
						snake::you.move(0, 1);

						if ((fast) || (k == repeat) || (you.line == lcnt - 1))
							snake::you.display(snake::screen);
					}
					break;
			}

			if (snake::you.intersects(&snake::money)) {
				loot += 25;

				if (k < repeat)
					snake::screen.print(' ', you.col, you.line, snake::BLACK);

				do {
					money = snake::money.warp(obstacles);
				} while ((money.col == finish.col &&
					money.line == finish.line) ||
				    (money.col < 5 && money.line == 0) ||
				    (money.col == you.col &&
					money.line == you.line));

				snake::money = snake::Treasure(money.col, money.line);
				snake::money.display(snake::screen);
				winnings(cashvalue);
				continue;
			}

			if (snake::you.intersects(&snake::finish)) {
				win(&finish);
				flushi();
				endwin();
				printf("You have won with $%d.\n", cashvalue);
				fflush(stdout);
				snake::log.write("won", cashvalue, ccnt, lcnt);
				post(cashvalue, 1);
				length(moves);
				exit(0);
			}

			if (pushsnake(obstacles))
				break;
		}
	}
}

/*
 * setup the board
 */
void setup()
{
	erase();
	std::vector<snake::ICurses*> to_show {&snake::finish, &snake::money, &snake::monster};
	setup(snake::screen, to_show);
	snake::room.display(snake::screen);
	refresh();
}

/**
 * Full screen refresh.
 */
void setup(snake::Screen screen, std::vector<snake::ICurses*> bodies)
{
	erase();

	for (snake::ICurses* body : bodies)
		for (struct cell piece : body->cells())
			screen.print(piece.symbol, piece.column, piece.row, piece.colour);

	refresh();
}

int post(int iscore, int flag)
{
	short   score = iscore;
	short   uid;
	short   oldbest = 0;
	short   allbwho = 0, allbscore = 0;
	struct passwd *p;

	/* I want to printf() the scores for terms that clear on cook(),
	 * but this routine also gets called with flag == 0 to see if
	 * the snake should wink.  If (flag) then we're at game end and
	 * can printf.
	 */
	/*
	 * Neg uid, 0, and 1 cannot have scores recorded.
	 */
	if ((uid = getuid()) <= 1) {
		if (flag)
			printf("No saved scores for uid %d.\n", uid);
		return (1);
	}
	if (rawscores < 0) {
		/* Error reported earlier */
		return (1);
	}
	/* Figure out what happened in the past */
	read(rawscores, &allbscore, sizeof(short));
	read(rawscores, &allbwho, sizeof(short));
	lseek(rawscores, uid * sizeof(short), SEEK_SET);
	read(rawscores, &oldbest, sizeof(short));
	if (!flag) {
		lseek(rawscores, 0, SEEK_SET);
		return (score > oldbest ? 1 : 0);
	}

	/* Update this jokers best */
	if (score > oldbest) {
		lseek(rawscores, uid * sizeof(short), SEEK_SET);
		write(rawscores, &score, sizeof(short));
		printf("You bettered your previous best of $%d\n", oldbest);
	} else
		printf("Your best to date is $%d\n", oldbest);

	/* See if we have a new champ */
	p = getpwuid(allbwho);
	if (p == NULL || score > allbscore) {
		lseek(rawscores, 0, SEEK_SET);
		write(rawscores, &score, sizeof(short));
		write(rawscores, &uid, sizeof(short));
		if (allbwho)
			printf("You beat %s's old record of $%d!\n",
			       p->pw_name, allbscore);
		else
			printf("You set a new record!\n");
	} else
		printf("The highest is %s with $%d\n", p->pw_name, allbscore);
	lseek(rawscores, 0, SEEK_SET);
	return (1);
}

/*
 * Flush typeahead to keep from buffering a bunch of chars and then
 * overshooting.  This loses horribly at 9600 baud, but works nicely
 * if the terminal gets behind.
 */
void flushi()
{
	tcflush(0, TCIFLUSH);
}

const int mx[8] = {
	0, 1, 1, 1, 0, -1, -1, -1
};
const int my[8] = {
	-1, -1, 0, 1, 1, 1, 0, -1
};

const float absv[8] = {
	1, 1.4, 1, 1.4, 1, 1.4, 1, 1.4
};
int     oldw = 0;

void chase(
	struct point* np,
	struct point* sp, // unclear; seems to be snakes "neck", next point right after its head
	std::vector<snake::IBody*> to_avoid
)
{
	/* this algorithm has bugs; otherwise the snake would get too good */
	struct point d;
	int     w, i, wt[8]; // 8 is the amount of surrounding points.
	double  v1, v2, vp, max;
	point(&d, you.col - sp->col, you.line - sp->line); // set d to vector from sp to you
	v1 = sqrt((double) (d.col * d.col + d.line * d.line)); // Pythagorean theorem, distance between sp and you
	w = 0;
	max = 0;

	for (i = 0; i < 8; i++) {
		vp = d.col * mx[i] + d.line * my[i];
		v2 = absv[i];

		if (v1 > 0)
			vp = ((double) vp) / (v1 * v2);
		else
			vp = 1.0;

		if (vp > max) {
			max = vp;
			w = i;
		}
	}

	for (i = 0; i < 8; i++) {
		point(&d, sp->col + mx[i], sp->line + my[i]);
		wt[i] = 0;

		/*
		 * Change to allow snake to eat you if you're on the money,
		 * otherwise, you can just crouch there until the snake goes
		 * away.  Not positive it's right.
		 *
		 * if (d.line == 0 && d.col < 5) continue;
		 */
		for (snake::IBody* obstacle : to_avoid)
			if (obstacle->occupies(d.col, d.line))
				goto next_point;

		wt[i] = i == w ? loot / 10 : 1;

		if (i == oldw)
			wt[i] += loot / 20;
		next_point:;
	}

	for (w = i = 0; i < 8; i++)
		w += wt[i];

	vp = ((random() >> 6) & 01777) % w;

	for (i = 0; i < 8; i++)
		if (vp < wt[i])
			break;
		else
			vp -= wt[i];

	if (i == 8) {
		printw("failure\n");
		i = 0;

		while (wt[i] == 0)
			i++;
	}

	oldw = w = i;
	point(np, sp->col + mx[w], sp->line + my[w]);
}

void spacewarp(int w, std::vector<snake::IBody*> obstacles)
{
	struct point p;
	int j;
	const char* str;

	you = snake::you.warp(obstacles);
	point(&p, COLS / 2 - 8, LINES / 2 - 1);

	if (p.col < 0)
		p.col = 0;

	if (p.line < 0)
		p.line = 0;

	if (w) {
		str = "BONUS!!!";
		loot = loot - penalty;
		penalty = 0;
	} else {
		str = "SPACE WARP!!!";
		penalty += loot / PENALTY;
	}

	for (j = 0; j < 3; j++) {
		erase();
		refresh();
		delay(5);
		snake::screen.print(str, p.col, p.line, snake::WHITE);
		refresh();
		delay(10);
	}

	setup();
	winnings(cashvalue);
}

void snap()
{
#if 0 /* This code doesn't really make sense.  */
	struct point p;

	if (you.line < 3) {
		mvaddch(1, you.col + 1, '-');
	}
	if (you.line > lcnt - 4) {
		mvaddch(lcnt, you.col + 1, '_');
	}
	if (you.col < 10) {
		mvaddch(you.line + 1, 1, '(');
	}
	if (you.col > ccnt - 10) {
		mvaddch(you.line + 1, ccnt, ')');
	}
#endif
	if (!stretch(&money))
		if (!stretch(&finish)) {
			snake::screen.print('?', you.col, you.line, snake::WHITE);
			refresh();
			delay(10);
			snake::you.display(snake::screen);
		}
#if 0
	if (you.line < 3) {
		point(&p, you.col, 0);
		chk(&p);
	}
	if (you.line > lcnt - 4) {
		point(&p, you.col, lcnt - 1);
		chk(&p);
	}
	if (you.col < 10) {
		point(&p, 0, you.line);
		chk(&p);
	}
	if (you.col > ccnt - 10) {
		point(&p, ccnt - 1, you.line);
		chk(&p);
	}
#endif
	refresh();
}

int stretch(const struct point* ps)
{
	struct point p;
	point(&p, you.col, you.line);

	if ((abs(ps->col - you.col) < (ccnt / 12)) && (you.line != ps->line)) {
		if (you.line < ps->line) {

			for (p.line = you.line + 1; p.line <= ps->line; p.line++)
				snake::screen.print('v', p.col, p.line, snake::WHITE);

			refresh();
			delay(10);

			for (; p.line > you.line; p.line--)
				chk(&p);

		} else {

			for (p.line = you.line - 1; p.line >= ps->line; p.line--)
				snake::screen.print('^', p.col, p.line, snake::WHITE);

			refresh();
			delay(10);

			for (; p.line < you.line; p.line++)
				chk(&p);
		}
		return (1);
	} else
		if ((abs(ps->line - you.line) < (lcnt/7)) && (you.col != ps->col)) {
			p.line = you.line;

			if (you.col < ps->col) {

				for (p.col = you.col + 1; p.col <= ps->col; p.col++)
					snake::screen.print('>', p.col, p.line, snake::WHITE);

				refresh();
				delay(10);

				for (; p.col > you.col; p.col--)
					chk(&p);

			} else {

				for (p.col = you.col - 1; p.col >= ps->col; p.col--)
					snake::screen.print('<', p.col, p.line, snake::WHITE);

				refresh();
				delay(10);

				for (; p.col < you.col; p.col++)
					chk(&p);

			}

			return (1);
		}

	return (0);
}

void surround(struct point* ps)
{
	int     j;

	if (ps->col == 0)
		ps->col++;

	if (ps->line == 0)
		ps->line++;

	if (ps->line == LINES - 1)
		ps->line--;

	if (ps->col == COLS - 1)
		ps->col--;

	snake::screen.print("/*\\", ps->col, ps->line, snake::WHITE);
	snake::screen.print("* *", ps->col, ps->line+1, snake::WHITE);
	snake::screen.print("\\*/", ps->col, ps->line+2, snake::WHITE);

	for (j = 0; j < 20; j++) {
		snake::screen.print('@', ps->col+1, ps->line+1, snake::WHITE);
		refresh();
		delay(1);
		snake::screen.print(' ', ps->col+1, ps->line+1, snake::BLACK);
		refresh();
		delay(1);
	}

	if (post(cashvalue, 0)) {
		snake::screen.print("   ", ps->col, ps->line, snake::WHITE);
		snake::screen.print("o.o", ps->col, ps->line+1, snake::WHITE);
		snake::screen.print("\\_/", ps->col, ps->line+2, snake::WHITE);
		refresh();
		delay(6);
		snake::screen.print("   ", ps->col, ps->line, snake::WHITE);
		snake::screen.print("o.-", ps->col, ps->line+1, snake::WHITE);
		snake::screen.print("\\_/", ps->col, ps->line+2, snake::WHITE);
		refresh();
		delay(6);
	}

	snake::screen.print("   ", ps->col, ps->line, snake::WHITE);
	snake::screen.print("o.o", ps->col, ps->line+1, snake::WHITE);
	snake::screen.print("\\_/", ps->col, ps->line+2, snake::WHITE);
	refresh();
	delay(6);
}

void win(const struct point* ps)
{
	struct point x;
	int     j, k;
	int     boxsize;	/* actually diameter of box, not radius */

	boxsize = fast ? 10 : 4;
	point(&x, ps->col, ps->line);

	for (j = 1; j < boxsize; j++) {

		for (k = 0; k < j; k++) {
			snake::screen.print('#', x.col, x.line, snake::WHITE);
			x.line--;
		}

		for (k = 0; k < j; k++) {
			snake::screen.print('#', x.col, x.line, snake::WHITE);
			x.col++;
		}

		j++;

		for (k = 0; k < j; k++) {
			snake::screen.print('#', x.col, x.line, snake::WHITE);
			x.line++;
		}

		for (k = 0; k < j; k++) {
			snake::screen.print('#', x.col, x.line, snake::WHITE);
			x.col--;
		}

		refresh();
		delay(1);
	}
}

int pushsnake(std::vector<snake::IBody*> obstacles)
{
	int     i, bonus;
	int     issame = 0;
	struct point tmp;

	/*
	 * My manual says times doesn't return a value.  Furthermore, the
	 * snake should get his turn every time no matter if the user is
	 * on a fast terminal with typematic keys or not.
	 * So I have taken the call to times out.
	 */
	for (i = 4; i >= 0; i--)

		if (same(&snake::snake[i], &snake::snake[5]))
			issame++;

	if (!issame)
		snake::screen.print(' ', snake::snake[5].col, snake::snake[5].line, snake::WHITE);

	/* Need the following to catch you if you step on the snake's tail */
	tmp.col = snake::snake[5].col;
	tmp.line = snake::snake[5].line;

	for (i = 4; i >= 0; i--)
		snake::snake[i + 1] = snake::snake[i];

	chase(&snake::snake[0], &snake::snake[1], std::vector<snake::IBody*> {&snake::money, &snake::room, &snake::finish});
	snake::monster.warp(snake::snake);
	snake::monster.display(snake::screen);

	if (snake::monster.intersects(&snake::you) || snake::you.occupies(tmp.col, tmp.line)) {
		surround(&you);
		i = (cashvalue) % 10;
		bonus = ((random() >> 8) & 0377) % 10;
		mvprintw(lcnt + 1, 0, "%d\n", bonus);
		refresh();
		delay(30);

		if (bonus == i) {
			spacewarp(1, obstacles);
			snake::log.write("bonus", cashvalue, ccnt, lcnt);
			flushi();
			return (1);
		}

		flushi();
		endwin();

		if (loot >= penalty) {
			printf("\nYou and your $%d have been eaten\n", cashvalue);
		} else {
			printf("\nThe snake ate you.  You owe $%d.\n", -cashvalue);
		}
		snake::log.write("eaten", cashvalue, ccnt, lcnt);
		length(moves);
		exit(0);
	}

	return (0);
}

int chk(const struct point* sp)
{
	int j;

	if (snake::money.occupies(sp->col, sp->line)) {
		snake::money.display(snake::screen);
		return (2);
	}

	if (snake::finish.occupies(sp->col, sp->line)) {
		snake::finish.display(snake::screen);
		return (3);
	}


	if (snake::monster.occupies(sp->col, sp->line)) {
		snake::monster.display(snake::screen);
		return (4);
	}

	if ((sp->col < 4) && (sp->line == 0)) {
		winnings(cashvalue);

		if ((you.line == 0) && (you.col < 4))
			snake::you.display(snake::screen);

		return (5);
	}

	if (snake::you.occupies(sp->col, sp->line)) {
		snake::you.display(snake::screen);
		return (1);
	}

	snake::screen.print(' ', sp->col, sp->line, snake::WHITE);
	return (0);
}

void winnings(int won)
{
	if (won > 0) {
		mvprintw(1, 1, "$%d", won);
	}
}

void stop(int dummy __attribute__((__unused__)))
{
	signal(SIGINT, SIG_IGN);
	endwin();
	length(moves);
	exit(0);
}

void suspend()
{
	endwin();
	kill(getpid(), SIGTSTP);
	refresh();
	winnings(cashvalue);
}

void length(int num)
{
	printf("You made %d moves.\n", num);
}
