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
#include <citro2d.h>
#include <stdbool.h>

#define MAXPENGUINS 100
#define PENGUINSPEED 2
#define SPRITE_SIZE 0.75f

#define BLOCKWIDTH 10
#define BLOCKHEIGHT 10

#define COLS 32
#define ROWS 24

#define WIDTH (COLS*BLOCKWIDTH)
#define HEIGHT (ROWS*BLOCKHEIGHT)

#define LINEMAXSTUCK 750

#define PERCENTREQUIRED 80
#define PERCENTBONUS 80
#define PERCENTEXTRABONUS 85

#define MAX_THEMES	16

#define OPTIONFILE "sdmc:/.icebreaker"
#define HISCOREFILE "sdmc:/.icebreaker.scores"

typedef struct {
    long basescore;
    long score;
    int level;
    int clearbonus;
    int lifebonus;
    int lives;
    int clear;
    bool dead;
} GameStats;

bool game_init(void);
void game_start(void);
bool game_update(u32 keys, touchPosition touch);
void game_draw_bottom(C3D_RenderTarget *bottom);
void game_draw_top(C3D_RenderTarget *top, float parallax);
void game_close(void);
