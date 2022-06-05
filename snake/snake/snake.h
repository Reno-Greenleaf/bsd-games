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
#ifndef SNAKE_H
#define SNAKE_H

#include "screen.h"
#include <vector>
#include <array>

struct point {
	int col, line;
};

namespace settings {
    extern int horizontalLimit;
    extern int verticalLimit;
}

namespace snake {
    class IBody
    {
        public:
            virtual bool occupies(int, int) = 0;
            virtual bool display(Screen) = 0;
            virtual bool intersects(IBody*) = 0;
            virtual struct point warp( // warp the body to a random place within given
                int, // horizontal
                int // and vertical limits
            ) = 0;
    };

    class IAI
    {
        public:
            virtual bool avoid(IBody*) = 0;
    };

    class IPoint
    {
        public:
            virtual IPoint* move(int, int) = 0;
    };
}

void mainloop(std::vector<snake::IBody*>);
void chase(struct point*, struct point*, std::vector<snake::IBody*>);
int chk(const struct point*); // redraw the given point
void flushi(void);
void length(int); // message about how many moves you made
struct point* point(struct point*, int, int); // create (fill) a point
int post(int, int);
int pushsnake(std::vector<snake::IBody*>);
void setup(void);
void snap(void);
void spacewarp(int, std::vector<snake::IBody*>); // warp to a free random point
void stop(int) __attribute__((__noreturn__)); // end the game
int stretch(const struct point*);
void surround(struct point*); // animation of snake catching you
void suspend(void); // pause, put the game to background/sleep mode
void win(const struct point*); // animation of victory/escape
void winnings(int); // update score (money collected)

#endif // end of include guard