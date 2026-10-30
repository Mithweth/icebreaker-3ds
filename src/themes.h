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

#define MAX_SPRITES	4

typedef struct
{
	u32 background;
	u32 normaltext;
	u8  boardfillminr;
	u8  boardfillming;
	u8  boardfillminb;
	u8  boardfillmaxr;
	u8  boardfillmaxg;
	u8  boardfillmaxb;
	u32 gridline;
	u32 gridhighlight;
	u32 line1;
	u32 line2;
	u32 menuhighlight;
	u32 gameovertext;
	u32 scorescrolltext;
	u32 bonusscrolltext;
	u32 textentrybox;
	u32 textentrytext;
	u32 progressbaroutline;
	u32 progressbarinner;
	u32 progressbarfill;
	u32 spritecharge;
	u32 shieldbackground;
} ThemeColorsType;

typedef struct {
	char name[64];
	ThemeColorsType color;
	C2D_Image spriteimage[MAX_SPRITES];
	C2D_Image teleportbitmap[MAX_SPRITES];
	size_t sprite_count;
	size_t teleport_sprite_count;
	C2D_Image particlebitmap;
	C2D_Image energybitmap;
	C2D_Image shieldbitmap;
	C2D_Image gridbitmap;
} Theme;

extern Theme theme;

bool themes_exists(const char* name);
bool themes_settheme(const char* themename, bool included);
bool themes_load(void);
void themes_next(void);
void themes_prev(void);
const char* themes_current_name(void);
