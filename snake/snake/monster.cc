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
#include "monster.h"

bool snake::Monster::warp(std::array<struct point, 6> positions)
{
	sections = positions;
	return true;
}

bool snake::Monster::display(Screen screen)
{
	screen.print('s', sections[1].col, sections[1].line, WHITE);
	screen.print('s', sections[2].col, sections[2].line, WHITE);
	screen.print('s', sections[3].col, sections[3].line, WHITE);
	screen.print('s', sections[4].col, sections[4].line, WHITE);
	screen.print('s', sections[5].col, sections[5].line, WHITE);
	screen.print('S', sections[0].col, sections[0].line, WHITE);
	return true;
}

bool snake::Monster::occupies(int column, int row)
{
	if (sections.empty())
		return false;

	for (int i = 0; i < 6; i++) {
		if (sections[i].col == column && sections[i].line == row)
			return true;
	}

	return false;
}

bool snake::Monster::intersects(IBody* body)
{
	if (sections.empty())
		return false;

	for (int i = 0; i < 6; i++) {
		if (body->occupies(sections[i].col, sections[i].line))
			return true;
	}

	return false;
}

struct point snake::Monster::warp(int horizontal, int vertical)
{
	struct point place;
	return place;
}

struct point snake::Monster::warp(std::vector<IBody*> obstacles)
{
	bool found;

	if (obstacles.empty())
	{
		sections[0].col = random() % settings::horizontalLimit;
		sections[0].line = random() % settings::verticalLimit;
	}

	while (true) {
		bool found = true;
		sections[0].col = random() % settings::horizontalLimit;
		sections[0].line = random() % settings::verticalLimit;

		for (IBody* obstacle : obstacles)
		{
			if (obstacle == this)
				continue;
			else if (obstacle->occupies(sections[0].col, sections[0].line))
			{
				found = false;
				break;
			}
		}

		if (found)
			break;
	}

	for (int i = 1; i < 6; i++)
		chase(&sections[i], &sections[i - 1], avoidables);

	return sections[0];
}

std::array<struct point, 6> snake::Monster::warp()
{
	return sections;
}

bool snake::Monster::avoid(snake::IBody* body)
{
	avoidables.push_back(body);
	return false;
}

std::vector<struct cell> snake::Monster::cells()
{
	std::vector<struct cell> result;
	struct cell head {sections[0].col, sections[0].line, 'S', snake::GREEN};
	result.push_back(head);

	for (int i = 1; i <= 5; i++) {
		struct cell next_cell {sections[i].col, sections[i].line, 's', snake::GREEN};
		result.push_back(next_cell);
	}

	return result;
}

void snake::Monster::rewind()
{
	current_cell = 5;
}

bool snake::Monster::has_more_cells()
{
    return current_cell >= 0;
}

struct cell snake::Monster::get_next_cell()
{
	struct point current_point = sections[current_cell];
	const char symbol = current_cell > 0 ? 's' : 'S';
    struct cell new_cell = {current_point.col, current_point.line, symbol, snake::WHITE};
    current_cell--;
    return new_cell;
}