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
#include <curses.h>
#include "screen.h"

bool snake::Screen::load()
{
	initscr();

	if (!has_colors()) {
		endwin();
		printf("Your terminal does not support color.\n");
		return false;
	}

	int colors[8] = {BLACK, RED, GREEN, YELLOW, BLUE, MAGENTA, CYAN, WHITE};
	start_color();

	for (int color : colors)
	{
		init_pair(color, color, COLOR_BLACK);
		init_pair(color+10, COLOR_BLACK, color); // backgrounds
	}

	return true;
}

bool snake::Screen::print(char character, int column, int row, int color)
{
	int pair = color;
	attron(COLOR_PAIR(pair));
	mvaddch(row, column, character);
	attroff(COLOR_PAIR(pair));
	return true;
}

bool snake::Screen::print(const char* text, int column, int row, int color)
{
	int pair = color;
	attron(COLOR_PAIR(pair));
	mvaddstr(row, column, text);
	attroff(COLOR_PAIR(pair));
	return true;
}

bool snake::Screen::fill(int column, int row, int color)
{
	int pair = color + 10;
	attron(COLOR_PAIR(pair));
	mvaddch(row, column, ' ');
	attroff(COLOR_PAIR(pair));
	return true;
}

bool snake::Screen::select(int column, int row)
{
	move(row, column);
	return true;
}