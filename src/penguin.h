// penguin.h
/*
* IceBreaker
* Copyright (c) 2000-2020 Matthew Miller <mattdm@mattdm.org>
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

#include <citro2d.h>

typedef struct
{
	C2D_Image image;
	int x;
	int y;
	int xdelta;
	int ydelta;
	int speedslower;
} Penguin;

Penguin penguin_create(void);
Penguin penguin_createxy(int x, int y);
// extern void penguin_resetimage(Penguin* p);
// extern void penguin_delete(Penguin* p);
// extern void penguin_savebehind(Penguin* p);
void penguin_draw(Penguin* p);
// extern void penguin_erase(Penguin* p);

typedef enum {
	NONE,
	LINE1,
	LINE2,
	BOTH 
} PenguinHit;

PenguinHit penguin_move(Penguin* p);
