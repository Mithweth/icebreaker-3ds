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

#include <stddef.h>
#include "line.h"

#define MAX_STATE_HISTORY	16

typedef enum {
	SLOW_NONE,
	SLOW_X,
	SLOW_Y,
	SLOW_AXIS_COUNT
} SlowAxis;

typedef struct {
	int x;
	int y;
	int xdelta;
	int ydelta;
	int speedslower;
	int phase;
} PenguinState;

typedef struct {
	PenguinState state;
	SlowAxis slow_axis;
	size_t current_image;
	size_t image_tick;
	PenguinState history[MAX_STATE_HISTORY];
	size_t history_count;
	size_t charge_tick;
} Penguin;

typedef enum {
	PENGUIN_HIT_NONE,
	PENGUIN_HIT_LINE1,
	PENGUIN_HIT_LINE2,
	PENGUIN_HIT_BOTH 
} PenguinHit;

Penguin penguin_create(void);
Penguin penguin_createxy(int x, int y);
void penguin_draw(Penguin* p);
PenguinHit penguin_move(Penguin* p, int level, const Line *line1, const Line *line2);
