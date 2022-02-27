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
#include <stdlib.h>
#include "me.h"


struct point snake::Me::warp(int horizontal, int vertical)
{
    struct point place;
    place.col = column = random() % settings::horizontalLimit;
    place.line = row = random() % settings::verticalLimit;
	return place;
}

bool snake::Me::display(Screen screen)
{
	screen.print('I', column, row, WHITE);
    screen.select(column, row);
	return true;
}

bool snake::Me::occupies(int column, int row)
{
	return column == this->column && row == this->row;
}

bool snake::Me::intersects(IBody* body)
{
	return body->occupies(column, row);
}

bool snake::Me::warp(struct point position)
{
    column = position.col;
    row = position.line;
    return true;
}

struct point snake::Me::warp(std::vector<IBody*> obstacles)
{
    struct point place;

    if (obstacles.empty())
    {
        column = place.col = random() % settings::horizontalLimit;
        row = place.line = random() % settings::verticalLimit;
        return place;
    }

    bool found = false;

    while (true) {
        column = random() % settings::horizontalLimit;
        row = random() % settings::verticalLimit;

        for (IBody* obstacle : obstacles)
        {
            if (obstacle->occupies(column, row) && obstacle != this)
            {
                break;
            }

            found = true;
        }

        if (found)
        {
            place.col = column;
            place.line = row;
            return place;
        }
    }
}