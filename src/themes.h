/*
* IceBreaker
* Copyright (c) 2002-2020 Matthew Miller <mattdm@mattdm.org>
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
* or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
* for more details.
*
* You should have received a copy of the GNU General Public License along
* with this program; if not, write to the Free Software Foundation, Inc., 59
* Temple Place, Suite 330, Boston, MA 02111-1307 USA
*
*/
#pragma once

#include <3ds.h>
#include <citro2d.h>

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
	u32 copyrighttext;
	u32 progressbaroutline;
	u32 progressbarinner;
	u32 progressbarfill;
} ThemeColorsType;

typedef struct {
	char name[64];
	ThemeColorsType color;
	C2D_Image spriteimage;
} Theme;

extern Theme theme;

bool themes_exists(const char* name);
int themes_settheme(char* themename);
bool themes_load(const char *directory);
void themes_next(void);
char* themes_current_name(void);