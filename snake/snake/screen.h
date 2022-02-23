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
#ifndef SCREEN_H
#define SCREEN_H

#include <map>
#include <curses.h>

namespace snake {
	const int BLACK = COLOR_BLACK;
	const int RED = COLOR_RED;
	const int GREEN = COLOR_GREEN;
	const int YELLOW = COLOR_YELLOW;
	const int BLUE = COLOR_BLUE;
	const int MAGENTA = COLOR_MAGENTA;
	const int CYAN = COLOR_CYAN;
	const int WHITE = COLOR_WHITE;


	class Screen // any layout, canvas, window, screen etc. to display stuff
	{
		public:
			bool load();
			bool print( // render single character
				char, // letter to display
				int, // column
				int, // row
				int // color
			);
			bool print( // render some text
				const char *, // the text
				int, // horizontal offset
				int, // vertical offset
				int // color
			);
			bool fill( // fill a cell with color
				int, // column
				int, // row
				int // color
			);
			bool select( // focus on position in the given
				int, // column and
				int // row
			);
	};
}

#endif // end of include guard