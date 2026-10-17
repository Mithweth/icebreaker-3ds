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

#define HISCORENUM 5

#include <3ds.h>
#include "game.h"

typedef struct {
	char name[50];
	long score;
} HighScore;

void hiscore_read(void);
bool hiscore_check_high(long thisgamescore);
HighScore* hiscore_get(int id);
void hiscore_add(const char* name, long score);
void hiscore_write(void);
void hiscore_dialog_init(void);
void hiscore_dialog_draw(void);
bool hiscore_dialog_update(u32 keys, touchPosition touch, const GameStats *stats);

