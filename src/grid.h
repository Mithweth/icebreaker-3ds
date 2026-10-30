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

#include <stdbool.h>

void grid_mark(int x, int y, int w, int h, char fillchar);
void grid_drawblocks(int wallx, int wally);
void grid_generate_colors(void);
void grid_checkempty(int x, int y);
char grid_at(int x, int y);
bool grid_is_open(char c);
void grid_reset(void);
int grid_countcleared(void);
bool grid_expand(int cols, int rows, const char tmpgrid[cols][rows]);
void grid_restore_base(void);
void grid_fill_empty_areas(void);
