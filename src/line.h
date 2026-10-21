/*
 * IceBreaker
 * Copyright (c) 2000-2020 Matthew Miller <mattdm@mattdm.org>
 * Copyright (c) 2026 Jean-Baptiste Langlois
 *
 * Original game by Matthew Miller.
 * Nintendo 3DS port by Jean-Baptiste Langlois (2026).
 *
 * <http://www.mattdm.org/icebreaker/>
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the Free
 * Software Foundation; either version 2 of the License, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
 * or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License
 * for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, see <https://www.gnu.org/licenses/>.
 */

#pragma once

#include <3ds.h>

typedef enum { UP, DOWN, LEFT, RIGHT } LineDir;	

typedef struct {
	int x;
	int y;
	int w;
	int h;
} Coordinate;

typedef struct
{
	int on;
	Coordinate geom;
	Coordinate mark;
	int end;
	LineDir dir;
	char id;
	int dead;
	int speedslower;
	int stuckcount;
} Line;

void line_start(Line * l, LineDir d, int x, int y, int end);
Line line_create(int linenum);
int line_move(Line * l);
void line_draw(const Line *l, u32 color);
void line_kill(Line * l);
