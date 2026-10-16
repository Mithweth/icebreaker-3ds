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
#include "constants.h"
#include "line.h"
#include "grid.h"
// #include "laundry.h"
// #include "sound.h"
// #include "globals.h"
#include "level.h"
// #include "status.h"
// #include "text.h"
// #include "dialog.h"
// #include "menu.h"
#include "options.h"
// #include "themes.h"
// #include "event.h"
// #include "delay.h"

#define GESTURE_THRESHOLD 12

static Line line1;
static Line line2;
static int penguincount=0;
static Penguin flock[MAXPENGUINS];
static bool gesture_active = false;
static touchPosition gesture_start;
static touchPosition gesture_last;
static int clear=0;
static int timepenalty;
static float scoremod;
static float bonusmod;
static bool done;
static int percentextrabonus;
static int percentbonus;

static void level_action_startclick(int mousex, int mousey, bool vertical)
{
	int x,y,xdif,ydif;

	x=(((mousex-BORDERLEFT)/BLOCKWIDTH ) * BLOCKWIDTH ) +BORDERLEFT;
	y=(((mousey-BORDERTOP)/BLOCKHEIGHT) * BLOCKHEIGHT) +BORDERTOP;
	xdif=mousex-x; ydif=mousey-y;

	if (grid[x][y] == '*' || grid[mousex][mousey] == '*') 
	{ // a little bit of grace if the player clicks directly on the penguin
		//sound_play(SNDOUCH);
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

bool level_init(GameStats *stats) {
	grid_generate_colors();
	grid_reset(' ');
	//LevelExitType returncode=ERROR;
	//int i,mx,my;

	scoremod=1.0f;
	bonusmod=1.0f;
	percentextrabonus=PERCENTEXTRABONUS-(stats->level-1)/5; // this needs to decrease, or else the bonus disintegrates as the space required to actually trap penguins increases
	percentbonus=PERCENTBONUS; // right now, there's really no point in making this a variable but it doesn't hurt, and keeps things consistant
	timepenalty=0;
	int timepenaltyinterval=0;
	done = false;

	penguincount=stats->level+1;
	stats->lives=stats->level+1;
	clear=0;
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
			return false;
		break;
	}
	stats->basescore = 0;
	stats->clearbonus=0;
	stats->lifebonus=0;
	line1=line_create(1);
	line2=line_create(2);

	for (int i=0;i<penguincount;i++)
	{
		flock[i] = penguin_create();
	}

	return true;
}

void level_draw(void) {
	grid_drawblocks();
	if (line1.on) {
		line_draw(&line1, line1.color);
	}
	if (line2.on) {
    	line_draw(&line2, line2.color);
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
			stats->basescore=stats->previousscore+(int)(stats->clear*scoremod)-timepenalty;
			//tick=0; // reset this to keep score from seeming to flicker around a lot. and to be nice. :)
		}

		if (line1.dead) 
		{
			//sound_play(SNDBREAK);
			line_kill(&line1);
			stats->lives--;
			if (stats->lives<0) stats->lives=0;
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
			stats->basescore=stats->previousscore+(int)(stats->clear*scoremod)-timepenalty;
			//tick=0; // reset this to keep score from seeming to flicker around a lot. and to be nice. :)
		}

		if (line2.dead)
		{ 
			//sound_play(SNDBREAK);
			line_kill(&line2);
			stats->lives--;
			if (stats->lives<0) stats->lives=0;
			//status_updatestats->lives(stats->lives);

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
		//returncode=PASS;


		stats->clearbonus=0;
		// bonuses for clearing lots
		if (stats->clear>PERCENTBONUS)
			stats->clearbonus+=(int)((stats->clear-percentbonus)*bonusmod);
		if (stats->clear>PERCENTEXTRABONUS)
			stats->clearbonus+=(int)((stats->clear-percentextrabonus)*(stats->clear-percentextrabonus)*bonusmod);
			
		// bonuses for leftover stats->lives
		stats->lifebonus=(int)((stats->lives-1)*3*bonusmod);
		//stats->score = stats->previousscore + stats->basescore + stats->clearbonus + stats->lifebonus;
		//stats->basescore = 0;
	} 

	return true;
}

void level_close(void) {
	return;
}


// 	do 
// 	{
// 		loopstartticks = SDL_GetTicks();
	
// 		// FIX -- this is way too messy. time to split it up into
// 		// neat little functions or something. Especially the menubutton stuff.
// 		while (event_poll(&event))
// 		{
// 			if (event.type == SDL_QUIT)
// 			{
// 				stats->lives=0; // is this the right way?
// 				done = true;
// 				// fix -- should delete penguins, etc. here
// 				return QUIT;
// 			}
// 			else if (event.type == SDL_MOUSEMOTION)
// 			{
// 				//if (grid[event.motion.x][event.motion.y] == ' ' || grid[event.motion.x][event.motion.y] == '*')
// 				if (event.motion.x>BORDERLEFT && event.motion.x<BORDERRIGHT && event.motion.y>BORDERTOP && event.motion.y<BORDERBOTTOM)	
// 				{
// 					switch (linetype)
// 					{
// 						case HORIZONTAL:
// 							cursor_set(CURSORHORIZONTAL);
// 						break;
// 						case VERTICAL:
// 							cursor_set(CURSORVERTICAL);
// 						break;
// 					}
// 				}			
// 				else  // we're somewhere outside of the playing area
// 				{
// 					cursor_set(CURSORARROW);
// 				}
				
// 				if (event.motion.x>=menubuttonrect.x && event.motion.y>=menubuttonrect.y && event.motion.x<menubuttonrect.x+menubuttonrect.w && event.motion.y<menubuttonrect.y+menubuttonrect.h)
// 				{ // over the menu button
// 					if (!menubuttonglow)
// 					{
// 						menubuttonglow=true;
// 						menu_drawbutton(&menubuttonrect,menubuttonglow);
// 					}
// 				}
// 				else 
// 				{
// 					if (menubuttonglow && !menubuttonpressed)
// 					{
// 						menubuttonglow=false;
// 						menu_drawbutton(&menubuttonrect,menubuttonglow);
// 					}
				
// 				}
// 			}
// 			else if (event.type == SDL_MOUSEBUTTONDOWN)
// 			{
// 				if (event.button.button==1) //left
// 				{
// 					// in game area?
// 					if (event.button.x>BORDERLEFT && event.button.x<BORDERRIGHT && event.button.y>BORDERTOP && event.button.y<BORDERBOTTOM)
// 					{
// 						level_action_startclick(event.button.x, event.button.y, linetype);
// 					}
// 					else if (event.button.x>=menubuttonrect.x && event.button.y>=menubuttonrect.y && event.button.x<menubuttonrect.x+menubuttonrect.w && event.button.y<menubuttonrect.y+menubuttonrect.h)
// 					{
// 						menubuttonpressed=true;
// 					}
// 				}
//   				else //middle or right
//   				{
//   					level_action_switchclick(&linetype);
// 				}
// 			}
// 			else if (event.type == SDL_MOUSEBUTTONUP)
// 			{
// 				if (menubuttonpressed && event.button.x>=menubuttonrect.x && event.button.y>=menubuttonrect.y && event.button.x<menubuttonrect.x+menubuttonrect.w && event.button.y<menubuttonrect.y+menubuttonrect.h)
// 				{
// 					menubuttonglow=true;
// 					menu_drawbutton(&menubuttonrect,menubuttonglow);
// 					domenuflag=true;
// 				}
// 				else if (menubuttonglow && menubuttonpressed)
// 				{
// 					menubuttonglow=false;
// 					menu_drawbutton(&menubuttonrect,menubuttonglow);
// 				}
// 				menubuttonpressed=false;
// 			}
// 			else if (event.type == SDL_KEYDOWN)
// 			{
			
// 				switch(translatekeyevent(&event))
// 				{
// 					case KEYMENU:
// 						domenuflag=true;
// 					break;
// 					case KEYPAUSE:
// 						// FIX -- this doesn't work.
// 						// paused=!paused;
// 					break;
// 					case KEYHELP: // fix -- kludgy code duplication
// 						cursor_set(CURSORARROW);
// 						if (dialog_popuphelp()==POPUPQUITGAME)
// 						{
// 							stats->lives=0; // is this the right way?
// 							done = true;
// 							return QUIT;
// 						}
// 						getmousestate(&mx,&my);
// 						if (mx>BORDERLEFT && mx<BORDERRIGHT && my>BORDERTOP && my<BORDERBOTTOM)	
// 						{
// 							switch (linetype)
// 							{
// 								case HORIZONTAL:
// 									cursor_set(CURSORHORIZONTAL);
// 								break;
// 								case VERTICAL:
// 									cursor_set(CURSORVERTICAL);
// 								break;
// 							}
// 						}
// 					break;
// 					case KEYSTARTLINE:
// 						// in game area?
// 						getmousestate(&mx,&my);
// 						if (mx>BORDERLEFT && mx<BORDERRIGHT && my>BORDERTOP && my<BORDERBOTTOM)
// 							level_action_startclick(mx, my, linetype);
// 						else if (menubuttonglow && mx>=menubuttonrect.x && my>=menubuttonrect.y && mx<menubuttonrect.x+menubuttonrect.w && my<menubuttonrect.y+menubuttonrect.h)
// 							domenuflag=true;
// 					break;
// 					case KEYSWITCHLINE:
// 	  					level_action_switchclick(&linetype);
// 					break;
// 					case KEYMOVELEFT:
// 						movecursorleftflag = true;
// 					break;
// 					case KEYMOVERIGHT:
// 						movecursorrightflag = true;
// 					break;
// 					case KEYMOVEUP:
// 						movecursorupflag = true;
// 					break;
// 					case KEYMOVEDOWN:
// 						movecursordownflag = true;
// 					break;
// 					case KEYMOVELEFTUP:
// 						movecursorleftflag = true; movecursorupflag = true;
// 					break;
// 					case KEYMOVERIGHTUP:
// 						movecursorrightflag = true; movecursorupflag = true;
// 					break;
// 					case KEYMOVELEFTDOWN:
// 						movecursorleftflag = true; movecursordownflag = true;
// 					break;
// 					case KEYMOVERIGHTDOWN:
// 						movecursorrightflag = true; movecursordownflag = true;
// 					break;
// 					default:
// 					break;
// 				}
// 			}
// 			else if (event.type == SDL_KEYUP)
// 			{
// 				switch(translatekeyevent(&event))
// 				{
// 					case KEYMOVELEFT:
// 						movecursorleftflag = false;
// 					break;
// 					case KEYMOVERIGHT:
// 						movecursorrightflag = false;
// 					break;
// 					case KEYMOVEUP:
// 						movecursorupflag = false;
// 					break;
// 					case KEYMOVEDOWN:
// 						movecursordownflag = false;
// 					break;
// 					case KEYMOVELEFTUP:
// 						movecursorleftflag = false; movecursorupflag = false;
// 					break;
// 					case KEYMOVERIGHTUP:
// 						movecursorrightflag = false; movecursorupflag = false;
// 					break;
// 					case KEYMOVELEFTDOWN:
// 						movecursorleftflag = false; movecursordownflag = false;
// 					break;
// 					case KEYMOVERIGHTDOWN:
// 						movecursorrightflag = false; movecursordownflag = false;
// 					break;
// 					default:
// 					break;
// 				}
// 			}
// 			else if (event.type == SDL_ACTIVEEVENT && ((options.autopause == AUTOPAUSEON) || (event.active.state & SDL_APPACTIVE)))
// 			{   
// 				if (event.active.gain==0)
// 				{ // iconified / lost focus
// 					paused=true;
// 				}
// 				else // event.active.gain==1
// 				{ // restored /got focus
// 					paused=false;
// 				}
// 			}
// 			// fix -- other events to handle?
// 		}
		
// 		// move the mouse cursor in response to arrow keys
// 		if (!keyboarddelay)
// 		{
// 			if (movecursorleftflag || movecursorrightflag || movecursorupflag || movecursordownflag)
// 			{
// 				getmousestate(&mx,&my);
// 				mx=((mx/BLOCKWIDTH ) * BLOCKWIDTH ) + 4; // these numbers are kludgy -- really ought to change based on margin width
// 				my=((my/BLOCKHEIGHT) * BLOCKHEIGHT) + 3;
				                
// 				if (movecursorleftflag)
// 				{
// 					if (mx>BORDERLEFT)
// 						mx-=BLOCKWIDTH;
// 					else
// 						movecursorleftflag=false;
// 				}
// 				if (movecursorrightflag)
// 				{
// 					if (mx<BORDERRIGHT)
// 						mx+=BLOCKWIDTH;
// 					else
// 						movecursorrightflag=false;
// 				}
// 				if (movecursorupflag)
// 				{
// 					if (my>BORDERTOP)
// 						my-=BLOCKHEIGHT;
// 					else
// 						movecursorupflag=false;
// 				}
// 				if (movecursordownflag)
// 				{	
// 					if (my<BORDERBOTTOM)
// 						my+=BLOCKHEIGHT;
// 					else
// 						movecursordownflag=false;
// 				}
// 				event_jumpmouse(mx,my);
// 				keyboarddelay=KEYBOARDARROWTHROTTLER;
// 				if (keyboardcounter>=KEYBOARDARROWACCELPOINT*3)
// 					keyboarddelay=KEYBOARDARROWTHROTTLER/3;
// 				else if (keyboardcounter>=KEYBOARDARROWACCELPOINT)
// 					keyboarddelay=KEYBOARDARROWTHROTTLER/2;
// 				keyboardcounter++;
					
				
// 			}
// 			else
// 			{
// 				keyboardcounter=0;
// 			}
// 		}
// 		else
// 		{
// 			keyboarddelay--;
// 		}
		
		
// 		// move split-lines
// 		if (line1.on)
// 		{
// 			// kludgy crap to implement speed = 1 1/2
// 			linedoneflag=line_move(&line1);
// 			if (!linedoneflag && line1.speedslower)
// 			{
// 				line1.speedslower=false;
// 				linedoneflag=line_move(&line1);
// 			}
// 			else
// 			{
// 				line1.speedslower=true;
// 			}
				
// 			if (linedoneflag)
// 			{
// 				clear=(100-(grid_countcleared()*100/(PLAYWIDTH*PLAYHEIGHT)));
// 				levelscore.basescore=(int)(clear*scoremod)-timepenalty;
// 				score=oldscore+levelscore.basescore;
// 				tick=0; // reset this to keep score from seeming to flicker around a lot. and to be nice. :)
				
// 				status_updatecleared(clear);
// 				status_updatescore(score);
// 				//printf("Cleared: %d%%\n",clear);				
// 			}

// 			if (line1.dead) 
// 			{
// 				sound_play(SNDBREAK);
// 				line_kill(&line1);
// 				stats->lives--;
// 				if (stats->lives<0) stats->lives=0;
// 				status_updatestats->lives(stats->lives);
// 				//printf("Lives: %d\n",stats->lives); 
// 			}
// 		}
// 		if (line2.on)
// 		{
// 			linedoneflag=line_move(&line2);
// 			if (!linedoneflag && line2.speedslower)
// 			{
// 				line2.speedslower=false;
// 				linedoneflag=line_move(&line2);
// 			}
// 			else
// 			{
// 				line2.speedslower=true;
// 			}

// 			if (linedoneflag)
// 			{
// 				clear=(100-(grid_countcleared()*100/(PLAYWIDTH*PLAYHEIGHT)));
// 				levelscore.basescore=(int)(clear*scoremod)-timepenalty;
// 				score=oldscore+levelscore.basescore;
// 				tick=0; // reset this to keep score from seeming to flicker around a lot. and to be nice. :)

// 				status_updatecleared(clear);
// 				status_updatescore(score);
// 				//printf("Cleared: %d%%\n",clear);
// 			}

// 			if (line2.dead)
// 			{ 
// 				sound_play(SNDBREAK);
// 				line_kill(&line2);
// 				stats->lives--;
// 				if (stats->lives<0) stats->lives=0;
// 				status_updatestats->lives(stats->lives);
// 				//printf("Lives: %d\n",stats->lives);
				
// 			}
// 		}
		
	
// 		// move (and get old background)
// 		for (i=0;i<penguincount;i++)
// 		{
			
// 			laundry_soil(flock[i].geom); // mark the penguin's old position as dirty
// 			penguin_move(&flock[i]);
		
// 			laundry_soil(flock[i].geom); // mark the penguin's new position as dirty too (it will be soon...)
// 			penguin_savebehind(&flock[i]);
// 		}
		

// 		// actually draw
// 		for (i=0;i<penguincount;i++)
// 		{
// 			penguin_draw(&flock[i]);
// 		}

// 		if (domenuflag)
// 		{
// 			cursor_set(CURSORARROW);
// 			switch (menu_popuplevel())
// 			{
// 				case POPUPQUITGAME:
// 					stats->lives=0; // is this the right way?
// 					done = true;
// 					return QUIT;
// 				break;
// 				case POPUPNEWGAME:
// 					// hmmm... maybe this could be done better
// 					done = true;
// 					if (level_checkifatstart())
// 						returncode=ZERO;
// 					else
// 						returncode=DEAD;
// 				break;
// 				default:
// 					// if we don't recognize the code, do nothing.
// 				break;
// 			}
			
// 			domenuflag=false;
// 		}

		
// 		// update clock
// 		tick++;
// 		if (tick>timepenaltyinterval)
// 		{
// 			tick=0;
// 			if (levelscore.basescore>0)
// 			{
// 				timepenalty++;
// 				levelscore.basescore--;
// 				score=oldscore+levelscore.basescore;
// 				status_updatescore(score);
// 			}
// 		}
		
// 		// update screen
// 		laundry_clean();

// 		if (paused) 
// 		{
// 			SDL_WaitEvent(NULL);
// 		}


// 		// clear for next update
// 		for (i=0;i<penguincount;i++)
// 		{
// 			penguin_erase(&flock[i]);
// 		}
		
// 		if (stats->lives<=0) // game over
// 		{
// 			done = true;
// 			returncode=DEAD;
// 		} 
// 		else if (!line1.on && !line2.on && clear>=PERCENTREQUIRED) // success!
// 		{ 
// 			done = true;
// 			levelscore.basescore=(int)(clear*scoremod)-timepenalty;
// 			returncode=PASS;


// 			levelscore.clearbonus=0;
// 			// bonuses for clearing lots
// 			if (clear>percentbonus)
// 				levelscore.clearbonus+=(int)((clear-percentbonus)*bonusmod);
// 			if (clear>percentextrabonus)
// 				levelscore.clearbonus+=(int)((clear-percentextrabonus)*(clear-percentextrabonus)*bonusmod);
				
// 			// bonuses for leftover stats->lives
// 			levelscore.lifebonus=(int)((stats->lives-1)*3*bonusmod);
// 		} 

// 		//grid_printboard();
		
// 		delay_til(loopstartticks+MSECPERFRAME);

		
// 	} while (!done);

// 	// make sure visible penguins are actually in the screen memory,
// 	// and then erase them so we don't leak surfaces.
// 	for (i=0;i<penguincount;i++)
// 	{
// 		penguin_draw(&flock[i]);
// 		penguin_delete(&flock[i]);
// 	}

// 	laundry_clean();
// 	return returncode;
// }



// // this function at least makes us able to keep the level variables local
// // to this file instead of totally global...
// void level_redrawwhole()
// {
// 	int i;
// 	SDL_FillRect(screen,NULL,color.background);
// 	grid_drawblocks();

// 	updatehiscorebox();
// 	status_updatescore(score);
// 	status_updatestats->lives(stats->lives);
// 	status_updatecleared(clear);
	
// 	// FIX -- changing theme makes line appear to grow
// 	// by one pixel. that's kinda gross -- we should correct that.
// 	if (line1.on) SDL_FillRect(screen,&(line1.geom),*(line1.colorpointer));
// 	if (line2.on) SDL_FillRect(screen,&(line2.geom),*(line2.colorpointer));
	
// 	for (i=0;i<penguincount;i++)
// 	{
// 		penguin_resetimage(&flock[i]);
// 		penguin_savebehind(&flock[i]);
// 		penguin_draw(&flock[i]);
// 	}

// }

// int level_checkifatstart(void)
// {
// 	return (clear==0 && score==0);
// }

// void level_action_startclick(int mousex, int mousey, LineType linetype)
// {
// 	int x,y,xdif,ydif;

// 	x=(((mousex-BORDERLEFT)/BLOCKWIDTH ) * BLOCKWIDTH ) +BORDERLEFT;
// 	y=(((mousey-BORDERTOP)/BLOCKHEIGHT) * BLOCKHEIGHT) +BORDERTOP;
// 	xdif=mousex-x; ydif=mousey-y;

// 	if (grid[x][y] == '*' || grid[mousex][mousey] == '*') 
// 	{ // a little bit of grace if the player clicks directly on the penguin
// 		sound_play(SNDOUCH);
// 	}
// 	else if (grid[x][y] == ' ')
// 	{
// 		switch (linetype)
// 		{
// 			case VERTICAL:
// 				if (!line1.on)
// 					(ydif<BLOCKHEIGHT/2) ? line_start(&line1,UP,x,y) : line_start(&line1,UP,x,y+BLOCKHEIGHT);
// 				if (!line2.on)
// 					(ydif<BLOCKHEIGHT/2) ? line_start(&line2,DOWN,x,y) : line_start(&line2,DOWN,x,y+BLOCKHEIGHT);
// 			break;
// 			case HORIZONTAL:
// 				if (!line1.on) 
// 					(xdif<BLOCKWIDTH/2) ? line_start(&line1,LEFT,x,y) : line_start(&line1,LEFT,x+BLOCKWIDTH,y);
// 				if (!line2.on)
// 					(xdif<BLOCKWIDTH/2) ? line_start(&line2,RIGHT,x,y) : line_start(&line2,RIGHT,x+BLOCKWIDTH,y);
// 			break;
// 		}
// 	}
// }

// void level_action_switchclick(LineType* linetypepointer)
// {
// 	switch (*linetypepointer)
// 	{
// 		case VERTICAL:
// 			*linetypepointer=HORIZONTAL;
// 			cursor_set(CURSORHORIZONTAL); 
// 		break;
// 		case HORIZONTAL:
// 			*linetypepointer=VERTICAL;
// 			cursor_set(CURSORVERTICAL);
// 		break;
// 	}
// }
