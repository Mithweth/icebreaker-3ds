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

#include <citro2d.h>
#include <stdio.h>
#include "game.h"
#include "line.h"
#include "grid.h"
#include "themes.h"

void line_draw(const Line *l, u32 color) {
    if (!l->on) return;
    C2D_DrawRectSolid(l->geom.x, l->geom.y, 0.4f, l->geom.w, l->geom.h, color);
}

static void line_finish(Line * l)
{
	int i;
	int quick1=false;
	int quick2=false;
	
	l->on=false;	
	
	//grid_printwhole();
	
	switch (l->dir)
  	{
  		case DOWN:
  		// falls through.
  		case UP:
  			grid_mark(l->geom.x,l->geom.y,l->geom.w,l->geom.h,'|');
  			line_draw(l,theme.color.background);
  				
  			// scan along edges to quickly determine if this
  			// is going to be complicated.
  			quick1=true; quick2=true;
  			for (i=l->geom.y+BLOCKHEIGHT/2;i<l->geom.y+l->geom.h;i++)
  			{
  				if (grid_at(l->geom.x-1,i) != ' ' && grid_at(l->geom.x-1,i) != '*') quick1=false;
  				if (grid_at(l->geom.x+BLOCKWIDTH,i) != ' ' && grid_at(l->geom.x+BLOCKWIDTH,i) != '*') quick2=false;
  			}
  				
  			//printf("Quick %d %d\n",quick1,quick2);
  				
  			grid_checkempty(l->geom.x-BLOCKWIDTH/2,l->geom.y+BLOCKHEIGHT/2);
  			grid_checkempty(l->geom.x+BLOCKWIDTH+BLOCKWIDTH/2/2,l->geom.y+BLOCKHEIGHT/2);
  		
  			if (!quick1)
  				for (i=l->geom.y+BLOCKHEIGHT/2+BLOCKHEIGHT;i<l->geom.y+l->geom.h;i+=BLOCKHEIGHT)
  					grid_checkempty(l->geom.x-BLOCKWIDTH/2,i);
  			if (!quick2)
  				for (i=l->geom.y+BLOCKHEIGHT/2+BLOCKHEIGHT;i<l->geom.y+l->geom.h;i+=BLOCKHEIGHT)
  					grid_checkempty(l->geom.x+BLOCKWIDTH+BLOCKWIDTH/2,i);
  		break;
  		case RIGHT:
  		// falls through
  		case LEFT:
  			grid_mark(l->geom.x,l->geom.y,l->geom.w,l->geom.h,'-');
  			line_draw(l,theme.color.background);
	
  			// scan along edges to quickly determine if this
  			// is going to be complicated.
  			quick1=true; quick2=true;
  			for (i=l->geom.x+BLOCKWIDTH/2;i<l->geom.x+l->geom.w;i++)
  			{
  				if (grid_at(i,l->geom.y-1) != ' ' && grid_at(i,l->geom.y-1) != '*') quick1=false;
  				if (grid_at(i,l->geom.y+BLOCKHEIGHT) != ' ' && grid_at(i,l->geom.y+BLOCKHEIGHT) != '*') quick2=false;
  			}

	  		grid_checkempty(l->geom.x+BLOCKWIDTH/2,l->geom.y-BLOCKHEIGHT/2);
	  		grid_checkempty(l->geom.x+BLOCKWIDTH/2,l->geom.y+BLOCKHEIGHT+BLOCKHEIGHT/2);
	  			
	  		if (!quick1)
	  			for (i=l->geom.x+BLOCKWIDTH/2+BLOCKWIDTH;i<l->geom.x+l->geom.w;i+=BLOCKWIDTH)
		  			grid_checkempty(i,l->geom.y-BLOCKHEIGHT/2);
	  			
	  		if (!quick2)
	  			for (i=l->geom.x+BLOCKWIDTH/2+BLOCKWIDTH;i<l->geom.x+l->geom.w;i+=BLOCKWIDTH)
		  			grid_checkempty(i,l->geom.y+BLOCKHEIGHT+BLOCKHEIGHT/2);
		break;
	}
}

Line line_create(int linenum) {
	Line l;
	l.id = '0' + linenum;
	l.on=false;
	l.dir=UP;
	l.geom.x=0;
	l.geom.y=0;
	l.geom.w=BLOCKWIDTH;
	l.geom.h=BLOCKHEIGHT;
	l.mark=l.geom;
	l.dead=false;
	l.speedslower=false;
	l.stuckcount=0;
	l.shield_enabled = false;
	l.shield_hit = false;
	return l;
}

void line_start(Line * l, LineDir d, int x, int y, int end) {
	l->on=true;
	l->dir=d;
	l->stuckcount=0;
	l->end = end;
	l->shield_enabled = false;
	l->shield_hit = false;

	switch (d)
	{
		case UP: 
			l->geom.w=BLOCKWIDTH;
			l->geom.h=1;
			l->geom.x=x;
			l->geom.y=y-1;
		break;
		case DOWN:
			l->geom.w=BLOCKWIDTH;
			l->geom.h=1;
			l->geom.x=x;
			l->geom.y=y;
		break;
		case LEFT: 
			l->geom.w=1;
			l->geom.h=BLOCKHEIGHT;
			l->geom.x=x-1;
			l->geom.y=y;
		break;
		case RIGHT:
			l->geom.w=1;
			l->geom.h=BLOCKHEIGHT;
			l->geom.x=x;
			l->geom.y=y;
		break;
	}
	l->mark = l->geom;
}

int line_move(Line * l)
{
	int finish=false;
	char check1;
	char check2;
	grid_mark(l->mark.x,l->mark.y,l->mark.w,l->mark.h,l->id);
	switch (l->dir)
	{
		case UP:
			check1=grid_at(l->geom.x,l->geom.y-1);
			check2=grid_at(l->geom.x+BLOCKWIDTH-1,l->geom.y-1);
			if (l->end && l->geom.y <= l->end) finish=true;
		break;
		case DOWN:
			check1=grid_at(l->geom.x,l->geom.y+l->geom.h);
			check2=grid_at(l->geom.x+BLOCKWIDTH-1,l->geom.y+l->geom.h);
			if (l->end && l->geom.y + l->geom.h >= l->end) finish=true;
		break;
		case LEFT:
			check1=grid_at(l->geom.x-1,l->geom.y);
			check2=grid_at(l->geom.x-1,l->geom.y+BLOCKHEIGHT-1);
			if (l->end && l->geom.x <= l->end) finish=true;
		break;
		case RIGHT:
			check1=grid_at(l->geom.x+l->geom.w,l->geom.y);
			check2=grid_at(l->geom.x+l->geom.w,l->geom.y+BLOCKHEIGHT-1);
			if (l->end && l->geom.x + l->geom.w >= l->end) finish=true;
		break;
		default: // this will never happen. really.
			fprintf(stderr,"Line has no direction. That shouldn't have happened.\n");
			check1='!';
			check2='!';
		break;
	}

	if (!finish) {
		if (check1 == ' ' && check2 == ' ')
		{ // next space is empty
			switch (l->dir)
			{
				case UP:
					l->geom.y--;
					l->geom.h++;
					l->mark.y--;
					l->mark.h=1;
				break;
				case DOWN:
					l->geom.h++; // increase length of line -- top stays same 
					l->mark.y+=l->mark.h;
					l->mark.h=1;
				break;
				case LEFT:
					l->geom.x--;
					l->geom.w++;
					l->mark.x--;
					l->mark.w=1;
				break;
				case RIGHT:
					l->geom.w++; // increase width of line -- left side stays same 
					l->mark.x+=l->mark.w;
					l->mark.w=1;
				break;
			}
		}
		else if ((check1 == '*' || check2 == '*')) { // hit a penguin. kills line.
			if (l->shield_enabled) {
				l->shield_hit = true;
			} else {
				l->dead = true;
			}
		} else if (check1 == '1' || check2 == '1' || check1 == '2' || check2 == '2') {
			if (l->stuckcount > LINEMAXSTUCK) {
				// FIX -- should play sound when this happens, to
				// let users know that this isn't a mistake.
				finish = true;
			} else {
				l->stuckcount++;
				// FIX: kludge-o-rama!!
				// this could work around the irritating thing where
				// a line gets started 'on top' of another line. but it
				// is totally repairing the symptom, not the bug. *sigh*
				//if (l->geom.w==1 || l->geom.h==1) finish=true;
			}
		} else { // hit something else
			finish=true;
		}
	}
	if (finish) {
		grid_mark(l->mark.x,l->mark.y,l->mark.w,l->mark.h,l->id);
		line_finish(l);
		return 1;
	}
	
	return 0;
}


void line_kill(Line * l)
{
	l->on=false;
	l->dead=false;
	
	switch (l->dir)
  	{
  		case DOWN:
		// falls through.
  		case UP:
  			grid_mark(l->geom.x,l->geom.y,l->geom.w,l->geom.h,' ');
  		break;
  		case RIGHT:
  		// falls through
  		case LEFT:
  			grid_mark(l->geom.x,l->geom.y,l->geom.w,l->geom.h,' ');
	  	break;
	}
}

bool line_shield_enabled(const Line *l) {
	return l->shield_enabled;
}

void line_enable_shield(Line *l, bool v) {
	l->shield_enabled = v;
}
