/*
 * IceBreaker
 * Copyright (c) 2000-2020 Matthew Miller <mattdm@mattdm.org>
 * Copyright (c) 2000-2020 Enrico Tassi <gareuselesinge@infinito.it>
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

#include <3ds.h>
#include <stdio.h>
#include <stdlib.h>
#include "game.h"
#include "grid.h"
#include "penguin.h"
#include "line.h"
#include "themes.h"

#define LXPOS(x) (BLOCKWIDTH * (x))
#define LYPOS(y) (BLOCKHEIGHT * (y))

static int letterstep;
static bool linedone1=false;
static bool linedone2=false;
static Line line1;
static Line line2;
static Penguin tux;
static Penguin flock[100];
static int penguincount;
static bool frozen;

void intro_init(void) {
	grid_reset();
	grid_generate_colors();
	letterstep = 0;
	line1=line_create(1);
	line2=line_create(2);
	tux = penguin_create();
	grid_mark(tux.state.x,tux.state.y,BLOCKWIDTH,BLOCKHEIGHT,'*');
	penguincount = 0;
	frozen = true;
	/// Here's the fun part where we set up the letters.
	//B
	flock[penguincount]=penguin_createxy(LXPOS(0),LYPOS(11)); penguincount++;
	flock[penguincount]=penguin_createxy(LXPOS(0),LYPOS(12)); penguincount++; 
	flock[penguincount]=penguin_createxy(LXPOS(0),LYPOS(13)); penguincount++;
	flock[penguincount]=penguin_createxy(LXPOS(0),LYPOS(14)); penguincount++;
	flock[penguincount]=penguin_createxy(LXPOS(0),LYPOS(15)); penguincount++;
	flock[penguincount]=penguin_createxy(LXPOS(0),LYPOS(16)); penguincount++;

	flock[penguincount]=penguin_createxy(LXPOS(1),LYPOS(11)); penguincount++;
	flock[penguincount]=penguin_createxy(LXPOS(2),LYPOS(11)); penguincount++;
	
	flock[penguincount]=penguin_createxy(LXPOS(1),LYPOS(13)); penguincount++;
	flock[penguincount]=penguin_createxy(LXPOS(2),LYPOS(13)); penguincount++;
	
	flock[penguincount]=penguin_createxy(LXPOS(1),LYPOS(16)); penguincount++;
	flock[penguincount]=penguin_createxy(LXPOS(2),LYPOS(16)); penguincount++;
	
	flock[penguincount]=penguin_createxy(LXPOS(3),LYPOS(12)); penguincount++;
	
	flock[penguincount]=penguin_createxy(LXPOS(3),LYPOS(14)); penguincount++;
	flock[penguincount]=penguin_createxy(LXPOS(3),LYPOS(15)); penguincount++;
	
	//R
	flock[penguincount]=penguin_createxy(LXPOS(5),LYPOS(11)); penguincount++;
	flock[penguincount]=penguin_createxy(LXPOS(5),LYPOS(12)); penguincount++;
	flock[penguincount]=penguin_createxy(LXPOS(5),LYPOS(13)); penguincount++;
	flock[penguincount]=penguin_createxy(LXPOS(5),LYPOS(14)); penguincount++;
	flock[penguincount]=penguin_createxy(LXPOS(5),LYPOS(15)); penguincount++;
	flock[penguincount]=penguin_createxy(LXPOS(5),LYPOS(16)); penguincount++;
	
	flock[penguincount]=penguin_createxy(LXPOS(6),LYPOS(11)); penguincount++;
	flock[penguincount]=penguin_createxy(LXPOS(7),LYPOS(11)); penguincount++;
	
	flock[penguincount]=penguin_createxy(LXPOS(6),LYPOS(13)); penguincount++;
	flock[penguincount]=penguin_createxy(LXPOS(7),LYPOS(13)); penguincount++;
	
	flock[penguincount]=penguin_createxy(LXPOS(8),LYPOS(12)); penguincount++;
	
	flock[penguincount]=penguin_createxy(LXPOS(8),LYPOS(14)); penguincount++;
	flock[penguincount]=penguin_createxy(LXPOS(8),LYPOS(15)); penguincount++;
	flock[penguincount]=penguin_createxy(LXPOS(8),LYPOS(16)); penguincount++;
	
	//E
	flock[penguincount]=penguin_createxy(LXPOS(10),LYPOS(11)); penguincount++;
	flock[penguincount]=penguin_createxy(LXPOS(10),LYPOS(12)); penguincount++;
	flock[penguincount]=penguin_createxy(LXPOS(10),LYPOS(13)); penguincount++;
	flock[penguincount]=penguin_createxy(LXPOS(10),LYPOS(14)); penguincount++;
	flock[penguincount]=penguin_createxy(LXPOS(10),LYPOS(15)); penguincount++;
	flock[penguincount]=penguin_createxy(LXPOS(10),LYPOS(16)); penguincount++;
	
	flock[penguincount]=penguin_createxy(LXPOS(11),LYPOS(11)); penguincount++;
	flock[penguincount]=penguin_createxy(LXPOS(12),LYPOS(11)); penguincount++;
	
	flock[penguincount]=penguin_createxy(LXPOS(11),LYPOS(13)); penguincount++;
	flock[penguincount]=penguin_createxy(LXPOS(12),LYPOS(13)); penguincount++;
	
	flock[penguincount]=penguin_createxy(LXPOS(11),LYPOS(16)); penguincount++;
	flock[penguincount]=penguin_createxy(LXPOS(12),LYPOS(16)); penguincount++;
	
	//A
	flock[penguincount]=penguin_createxy(LXPOS(14),LYPOS(12)); penguincount++;
	flock[penguincount]=penguin_createxy(LXPOS(14),LYPOS(13)); penguincount++;
	flock[penguincount]=penguin_createxy(LXPOS(14),LYPOS(14)); penguincount++;
	flock[penguincount]=penguin_createxy(LXPOS(14),LYPOS(15)); penguincount++;
	flock[penguincount]=penguin_createxy(LXPOS(14),LYPOS(16)); penguincount++;
	
	flock[penguincount]=penguin_createxy(LXPOS(17),LYPOS(12)); penguincount++;
	flock[penguincount]=penguin_createxy(LXPOS(17),LYPOS(13)); penguincount++;
	flock[penguincount]=penguin_createxy(LXPOS(17),LYPOS(14)); penguincount++;
	flock[penguincount]=penguin_createxy(LXPOS(17),LYPOS(15)); penguincount++;
	flock[penguincount]=penguin_createxy(LXPOS(17),LYPOS(16)); penguincount++;

	flock[penguincount]=penguin_createxy(LXPOS(15),LYPOS(11)); penguincount++;
	flock[penguincount]=penguin_createxy(LXPOS(16),LYPOS(11)); penguincount++;

	flock[penguincount]=penguin_createxy(LXPOS(15),LYPOS(13)); penguincount++;
	flock[penguincount]=penguin_createxy(LXPOS(16),LYPOS(13)); penguincount++;

	//K
	flock[penguincount]=penguin_createxy(LXPOS(19),LYPOS(11)); penguincount++;
	flock[penguincount]=penguin_createxy(LXPOS(19),LYPOS(12)); penguincount++;
	flock[penguincount]=penguin_createxy(LXPOS(19),LYPOS(13)); penguincount++;
	flock[penguincount]=penguin_createxy(LXPOS(19),LYPOS(14)); penguincount++;
	flock[penguincount]=penguin_createxy(LXPOS(19),LYPOS(15)); penguincount++;
	flock[penguincount]=penguin_createxy(LXPOS(19),LYPOS(16)); penguincount++;
	
	flock[penguincount]=penguin_createxy(LXPOS(20),LYPOS(13)); penguincount++;
	flock[penguincount]=penguin_createxy(LXPOS(21),LYPOS(12)); penguincount++;
	flock[penguincount]=penguin_createxy(LXPOS(22),LYPOS(11)); penguincount++;
	
	flock[penguincount]=penguin_createxy(LXPOS(21),LYPOS(14)); penguincount++;
	flock[penguincount]=penguin_createxy(LXPOS(22),LYPOS(15)); penguincount++;
	flock[penguincount]=penguin_createxy(LXPOS(22),LYPOS(16)); penguincount++;

	//E
	flock[penguincount]=penguin_createxy(LXPOS(24),LYPOS(11)); penguincount++;
	flock[penguincount]=penguin_createxy(LXPOS(24),LYPOS(12)); penguincount++;
	flock[penguincount]=penguin_createxy(LXPOS(24),LYPOS(13)); penguincount++;
	flock[penguincount]=penguin_createxy(LXPOS(24),LYPOS(14)); penguincount++;
	flock[penguincount]=penguin_createxy(LXPOS(24),LYPOS(15)); penguincount++;
	flock[penguincount]=penguin_createxy(LXPOS(24),LYPOS(16)); penguincount++;
	
	flock[penguincount]=penguin_createxy(LXPOS(25),LYPOS(11)); penguincount++;
	flock[penguincount]=penguin_createxy(LXPOS(26),LYPOS(11)); penguincount++;
	
	flock[penguincount]=penguin_createxy(LXPOS(25),LYPOS(13)); penguincount++;
	flock[penguincount]=penguin_createxy(LXPOS(26),LYPOS(13)); penguincount++;
	
	flock[penguincount]=penguin_createxy(LXPOS(25),LYPOS(16)); penguincount++;
	flock[penguincount]=penguin_createxy(LXPOS(26),LYPOS(16)); penguincount++;

	//R
	flock[penguincount]=penguin_createxy(LXPOS(28),LYPOS(11)); penguincount++;
	flock[penguincount]=penguin_createxy(LXPOS(28),LYPOS(12)); penguincount++;
	flock[penguincount]=penguin_createxy(LXPOS(28),LYPOS(13)); penguincount++;
	flock[penguincount]=penguin_createxy(LXPOS(28),LYPOS(14)); penguincount++;
	flock[penguincount]=penguin_createxy(LXPOS(28),LYPOS(15)); penguincount++;
	flock[penguincount]=penguin_createxy(LXPOS(28),LYPOS(16)); penguincount++;
	
	flock[penguincount]=penguin_createxy(LXPOS(29),LYPOS(11)); penguincount++;
	flock[penguincount]=penguin_createxy(LXPOS(30),LYPOS(11)); penguincount++;
	
	flock[penguincount]=penguin_createxy(LXPOS(29),LYPOS(13)); penguincount++;
	flock[penguincount]=penguin_createxy(LXPOS(30),LYPOS(13)); penguincount++;
	
	flock[penguincount]=penguin_createxy(LXPOS(31),LYPOS(12)); penguincount++;
	
	flock[penguincount]=penguin_createxy(LXPOS(31),LYPOS(14)); penguincount++;
		flock[penguincount]=penguin_createxy(LXPOS(31),LYPOS(15)); penguincount++;
	flock[penguincount]=penguin_createxy(LXPOS(31),LYPOS(16)); penguincount++;

	if (penguincount>=MAXPENGUINS) { fprintf(stderr,"Too many penguins!\n"); }
}

void intro_draw(void) {
	grid_drawblocks();
	if (letterstep>11 || !frozen) {
		for (int i=0;i<penguincount;i++) {
			penguin_draw(&flock[i]);
		}
	}
	penguin_draw(&tux);
    line_draw(&line1, theme.color.line1);
    line_draw(&line2, theme.color.line2);
}

bool intro_update(u32 keys) {
	if (keys & KEY_TOUCH) {
		return false;
	}
	if (letterstep < 10)
	{	
		switch (letterstep)
		{
			// I
			case 0:
				if (!linedone1 && !line1.on) line_start(&line1,LEFT,LXPOS(2),LYPOS(2),LXPOS(1));
				if (!linedone2 && !line2.on) line_start(&line2,RIGHT,LXPOS(2),LYPOS(2),LXPOS(4));
			break;
			case 1:
				if (!linedone1 && !line1.on) line_start(&line1,LEFT,LXPOS(2),LYPOS(7),LXPOS(1));
				if (!linedone2 && !line2.on) line_start(&line2,RIGHT,LXPOS(2),LYPOS(7),LXPOS(4));
			break;
			case 2:
				if (!linedone1 && !line1.on) line_start(&line1,UP,LXPOS(2),LYPOS(5),LYPOS(3));
				if (!linedone2 && !line2.on) line_start(&line2,DOWN,LXPOS(2),LYPOS(5),LYPOS(7));
			break;
			// C
			case 3:
				if (!linedone1 && !line1.on) line_start(&line1,LEFT,LXPOS(7),LYPOS(2),LXPOS(5));
				if (!linedone2 && !line2.on) line_start(&line2,RIGHT,LXPOS(7),LYPOS(2),LXPOS(9));
			break;
			case 4:
				if (!linedone1 && !line1.on) line_start(&line1,UP,LXPOS(5),LYPOS(5),LYPOS(3));
				if (!linedone2 && !line2.on) line_start(&line2,DOWN,LXPOS(5),LYPOS(5),LYPOS(8));
			break;
			case 5:
				if (!linedone1 && !line1.on) line_start(&line1,LEFT,LXPOS(7),LYPOS(7),LXPOS(6));
				if (!linedone2 && !line2.on) line_start(&line2,RIGHT,LXPOS(7),LYPOS(7),LXPOS(9));
			break;
			// E
			case 6:
				if (!linedone1 && !line1.on) line_start(&line1,LEFT,LXPOS(11),LYPOS(2),LXPOS(10));
				if (!linedone2 && !line2.on) line_start(&line2,RIGHT,LXPOS(11),LYPOS(2),LXPOS(14));
			break;
			case 7:
				if (!linedone1 && !line1.on) line_start(&line1,UP,LXPOS(10),LYPOS(5),LYPOS(3));
				if (!linedone2 && !line2.on) line_start(&line2,DOWN,LXPOS(10),LYPOS(5),LYPOS(8));
			break;
			case 8:
				if (!linedone1 && !line1.on) line_start(&line1,LEFT,LXPOS(13),LYPOS(7),LXPOS(11));
				if (!linedone2 && !line2.on) line_start(&line2,RIGHT,LXPOS(13),LYPOS(7),LXPOS(14));
			break;
			case 9:
				if (!linedone1 && !line1.on) line_start(&line1,LEFT,LXPOS(12),LYPOS(4),LXPOS(11));
				if (!linedone2 && !line2.on) line_start(&line2,RIGHT,LXPOS(12),LYPOS(4),LXPOS(13));
			break;
		}
	}
	else if (letterstep==10)
	{
		letterstep++;
	}
	else if (letterstep==11)
	{
		for (int i=0;i<penguincount;i++)
		{
			grid_mark(flock[i].state.x,flock[i].state.y,BLOCKWIDTH,BLOCKHEIGHT,'*');
		}
		
		letterstep++;
	}
	else if (letterstep<300)
	{
		letterstep++;
	}
	else if (letterstep==300)
	{
		frozen=false;
		for (int i=0;i<penguincount;i++)
		{
			grid_mark(flock[i].state.x,flock[i].state.y,BLOCKWIDTH,BLOCKHEIGHT,' ');
		}
		letterstep++;
	}
	else if (letterstep<400)
	{
		letterstep++;
	}
	else if (letterstep==400)
	{
		if (penguincount)
		{
			penguincount--;
			grid_mark(flock[penguincount].state.x,flock[penguincount].state.y,BLOCKWIDTH,BLOCKHEIGHT,' ');
		}
		else
		{
			letterstep++;
		}
	}
	else if (letterstep<800)
	{
		if (!line1.on && !line2.on)
		{
			int x=LXPOS(random() % COLS);
			int y=LYPOS(random() % ROWS);
			if (random()%2)
			{
				line_start(&line1, UP, x, y, 0);
				line_start(&line2, DOWN, x, y, 0);
			}
			else
			{
				line_start(&line1, LEFT, x, y, 0);
				line_start(&line2, RIGHT, x, y, 0);
			}

		}
	}

	if (letterstep>11)
		for (int i=0;i<penguincount;i++)
		{
			grid_mark(flock[i].state.x,flock[i].state.y,BLOCKWIDTH,BLOCKHEIGHT,'*');
		}

	if (line1.on) {
		if (line_move(&line1))
		{
			linedone1=true;
		}
		if (line1.dead) // need a second check because line_move could change this
		{
			//no sound in intro?
			//sound_play(SNDBREAK);
			line_kill(&line1);
			linedone1=false;
		}
	}
	if (line2.on) {
	
		if (line_move(&line2))
		{
			linedone2=true;
		}
		if (line2.dead)
		{ 
			// no sound in intro?
			//sound_play(SNDBREAK);
			line_kill(&line2);
			linedone2=false;
		}
	}
	

	if (linedone1 && linedone2) { letterstep++; linedone1=false; linedone2=false; }
	
	if (!frozen) {
		for (int i=0;i<penguincount;i++) {
			PenguinHit hit = penguin_move(&flock[i]);
			if (hit == PENGUIN_HIT_LINE1 || hit == PENGUIN_HIT_BOTH) line1.dead=true;
			if (hit == PENGUIN_HIT_LINE2 || hit == PENGUIN_HIT_BOTH) line2.dead=true;
		}
	}
	PenguinHit hit = penguin_move(&tux);
	if (hit == PENGUIN_HIT_LINE1 || hit == PENGUIN_HIT_BOTH) line1.dead=true;
	if (hit == PENGUIN_HIT_LINE2 || hit == PENGUIN_HIT_BOTH) line2.dead=true;
	return true;
}

void intro_close(void) {
	return;
}
