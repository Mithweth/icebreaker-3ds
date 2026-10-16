/*
* IceBreaker
* Copyright (c) 2000-2020 Matthew Miller <mattdm@mattdm.org> and
*   Enrico Tassi <gareuselesinge@infinito.it>
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
* with this program; if not, write to the Free Software Foundation, Inc.,
* 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
*
*/

/* special thanks to Enrico for the grid coloring stuff for themes */

// #include <SDL.h>
// #include <stdio.h>

// #include "icebreaker.h"
// #include "cursor.h"
// #include "penguin.h"
#include <3ds.h>
#include <stdlib.h>
#include "game.h"
#include "line.h"
#include "grid.h"
#include "level.h"
#include "options.h"
#include "themes.h"
#include "sound.h"

#define GESTURE_THRESHOLD 12

static Line line1;
static Line line2;
static int penguincount=0;
static Penguin flock[MAXPENGUINS];
static bool gesture_active = false;
static touchPosition gesture_start;
static touchPosition gesture_last;
static int timepenalty;
static int timepenaltyinterval;
static float scoremod;
static float bonusmod;
static bool done;
static int percentextrabonus;
static int percentbonus;
static u64 now;

static void level_action_startclick(int mousex, int mousey, bool vertical)
{
	int x,y,xdif,ydif;

	x=(((mousex-BORDERLEFT)/BLOCKWIDTH ) * BLOCKWIDTH ) +BORDERLEFT;
	y=(((mousey-BORDERTOP)/BLOCKHEIGHT) * BLOCKHEIGHT) +BORDERTOP;
	xdif=mousex-x; ydif=mousey-y;

	if (grid[x][y] == '*' || grid[mousex][mousey] == '*') 
	{ // a little bit of grace if the player clicks directly on the penguin
		sound_play(SNDOUCH);
	}
	else if (grid[x][y] == ' ')
	{
		if (vertical)
		{
			if (!line1.on)
				(ydif<BLOCKHEIGHT/2) ? line_start(&line1,UP,x,y) : line_start(&line1,UP,x,y+BLOCKHEIGHT);
			if (!line2.on)
				(ydif<BLOCKHEIGHT/2) ? line_start(&line2,DOWN,x,y) : line_start(&line2,DOWN,x,y+BLOCKHEIGHT);
		} else {
			if (!line1.on) 
				(xdif<BLOCKWIDTH/2) ? line_start(&line1,LEFT,x,y) : line_start(&line1,LEFT,x+BLOCKWIDTH,y);
			if (!line2.on)
				(xdif<BLOCKWIDTH/2) ? line_start(&line2,RIGHT,x,y) : line_start(&line2,RIGHT,x+BLOCKWIDTH,y);
		}
	}
}

void level_init(GameStats *stats) {
	grid_generate_colors();
	grid_reset(' ');

	scoremod=1.0f;
	bonusmod=1.0f;
	percentextrabonus=PERCENTEXTRABONUS-(stats->level-1)/5; // this needs to decrease, or else the bonus disintegrates as the space required to actually trap penguins increases
	percentbonus=PERCENTBONUS; // right now, there's really no point in making this a variable but it doesn't hurt, and keeps things consistant
	timepenalty=0;
	done = false;

	penguincount=stats->level+1;
	stats->lives=stats->level+1;
	//stats->currentscore = stats->basescore;
	
	switch(options.difficulty)
	{
		case NORMAL:
			scoremod=(stats->level+1)/2.0f;
			bonusmod=(stats->level+1)/2.0f;
			timepenaltyinterval=100;
		break;
		case EASY:
			scoremod=(stats->level+2)/6.0f;
			bonusmod=(stats->level+1)/9.0f;
			timepenaltyinterval=200;
		break;
		case HARD:
			scoremod=(stats->level+1)/1.75f;
			bonusmod=(stats->level+1)/1.5f;
			timepenaltyinterval=75;
		break;
		default:
			fprintf(stderr,"Unknown difficulty -- that can't happen!\n");
			return;
		break;
	}
	stats->basescore = 0;
	stats->clearbonus=0;
	stats->lifebonus=0;
	stats->clear = 0;
	line1=line_create(1);
	line2=line_create(2);

	for (int i=0;i<penguincount;i++)
	{
		flock[i] = penguin_create();
	}
	now = osGetTime();
}

void level_draw(void) {
	grid_drawblocks();
	if (line1.on) {
		line_draw(&line1, theme.color.line1);
	}
	if (line2.on) {
    	line_draw(&line2, theme.color.line2);
	}
	for (int i=0;i<penguincount;i++) {
		penguin_draw(&flock[i]);
	}
}

bool level_update(u32 keys, touchPosition touch, GameStats *stats) {
	if (done) {
		return false;
	}
    if (keys & KEY_TOUCH) {
        gesture_start = touch;
        gesture_last = touch;
        gesture_active = true;
    }

    if (gesture_active && (hidKeysHeld() & KEY_TOUCH)) {
        gesture_last = touch;
    }

    if (gesture_active && (hidKeysUp() & KEY_TOUCH)) {
        int dx = (int)gesture_last.px - (int)gesture_start.px;
        int dy = (int)gesture_last.py - (int)gesture_start.py;

        if (abs(dx) >= GESTURE_THRESHOLD || abs(dy) >= GESTURE_THRESHOLD) {
            bool vertical = abs(dy) > abs(dx);

            level_action_startclick(gesture_start.px, gesture_start.py, vertical);
        }

        gesture_active = false;
    }

	if (line1.on)
	{
		// kludgy crap to implement speed = 1 1/2
		bool linedoneflag=line_move(&line1);
		if (!linedoneflag && line1.speedslower)
		{
			line1.speedslower=false;
			linedoneflag=line_move(&line1);
		}
		else
		{
			line1.speedslower=true;
		}
			
		if (linedoneflag)
		{
			stats->clear=(100-(grid_countcleared()*100/(PLAYWIDTH*PLAYHEIGHT)));
			stats->basescore=(int)(stats->clear*scoremod)-timepenalty;
			//tick=0; // reset this to keep score from seeming to flicker around a lot. and to be nice. :)
		}

		if (line1.dead) 
		{
			sound_play(SNDBREAK);
			line_kill(&line1);
			stats->lives--;
			if (stats->lives < 0) {
				stats->lives = 0;
				stats->dead = true;
				done = true;
			}
			//status_updatestats->lives(stats->lives);
		}
	}
	if (line2.on)
	{
		bool linedoneflag=line_move(&line2);
		if (!linedoneflag && line2.speedslower)
		{
			line2.speedslower=false;
			linedoneflag=line_move(&line2);
		}
		else
		{
			line2.speedslower=true;
		}

		if (linedoneflag)
		{
			stats->clear=(100-(grid_countcleared()*100/(PLAYWIDTH*PLAYHEIGHT)));
			stats->basescore=(int)(stats->clear*scoremod)-timepenalty;
		}

		if (line2.dead)
		{ 
			sound_play(SNDBREAK);
			line_kill(&line2);
			stats->lives--;
			if (stats->lives<0) stats->lives=0;

		}
	}
	for (int i=0;i<penguincount;i++)
	{
		PenguinHit hit = penguin_move(&flock[i]);
		if (hit == LINE1 || hit == BOTH) line1.dead=true;
		if (hit == LINE2 || hit == BOTH) line2.dead=true;
	}

	if (!line1.on && !line2.on && stats->clear>=PERCENTREQUIRED) // success!
	{ 
		done = true;
		stats->basescore=(int)(stats->clear*scoremod)-timepenalty;

		stats->clearbonus=0;
		// bonuses for clearing lots
		if (stats->clear>PERCENTBONUS)
			stats->clearbonus+=(int)((stats->clear-percentbonus)*bonusmod);
		if (stats->clear>PERCENTEXTRABONUS)
			stats->clearbonus+=(int)((stats->clear-percentextrabonus)*(stats->clear-percentextrabonus)*bonusmod);
			
		// bonuses for leftover stats->lives
		stats->lifebonus=(int)((stats->lives-1)*3*bonusmod);
	} 

	u64 elapsed = osGetTime();
	if (elapsed - now > timepenaltyinterval * 10) {
		now += timepenaltyinterval * 10;
		if (stats->basescore>0)
		{
			timepenalty++;
			stats->basescore=(int)(stats->clear*scoremod)-timepenalty;
		}
	}

	return true;
}

void level_close(void) {
	return;
}
