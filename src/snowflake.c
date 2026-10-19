/*
 * IceBreaker - Nintendo 3DS port
 * Copyright (c) 2026 Jean-Baptiste Langlois
 *
 * This file is part of IceBreaker.
 *
 * <https://github.com/Mithweth/icebreaker-3ds>
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

#define SNOWFLAKE_COUNT	30

#include <stdlib.h>
#include <citro2d.h>
#include "themes.h"

typedef struct {
	float x;
	float y;
	float speed;
	float drift;
	float depth;
	float size;
} Snowflake;

static Snowflake snowflakes[SNOWFLAKE_COUNT];

void snowflake_init(void) {
	for (int i = 0; i < SNOWFLAKE_COUNT; i++) {
		snowflakes[i].depth = (rand() % 100) / 100.0f - 1.0f;
		snowflakes[i].size = 0.5f + (snowflakes[i].depth + 1.0f) * 0.5f;
		snowflakes[i].speed = 0.2f + (snowflakes[i].depth + 1.0f) * 0.6f;
		snowflakes[i].drift = ((rand() % 100) / 100.0f - 0.5f) * 0.3f;
		snowflakes[i].y = -3.0f;
		snowflakes[i].x = rand() % 400;
	}
}

void snowflake_update(void) {
	for (int i = 0; i < SNOWFLAKE_COUNT; i++) {
		snowflakes[i].x += snowflakes[i].drift;
		snowflakes[i].y += snowflakes[i].speed;
		if (snowflakes[i].y > 240.0f) {
			snowflakes[i].y = -3.0f;
			snowflakes[i].x = rand() % 400;
		}
	}
}

void snowflake_draw(float parallax) {
	if (!theme.fallingparticle.tex) {
		return;
	}   
	for (int i = 0; i < SNOWFLAKE_COUNT; i++) {
		float x = snowflakes[i].x + snowflakes[i].depth * parallax;
		C2D_DrawImageAt(theme.fallingparticle, x, snowflakes[i].y, 0.1f, NULL, snowflakes[i].size, snowflakes[i].size);
	}
}
