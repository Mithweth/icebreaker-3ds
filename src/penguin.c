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

#include <stdlib.h>
#include "game.h"
#include "penguin.h"
#include "themes.h"
#include "grid.h"
#include "options.h"

Penguin penguin_create(void)
{
	return penguin_createxy(BORDERLEFT + (random() % (PLAYWIDTH-BLOCKWIDTH)),BORDERTOP + (random() % (PLAYHEIGHT-BLOCKWIDTH)));
}

Penguin penguin_createxy(int x, int y)
{
	Penguin p;

	switch (random() % 4)
	{
		case 0:
			p.xdelta= PENGUINSPEED;p.ydelta= PENGUINSPEED;
		break;
		case 1:
			p.xdelta= PENGUINSPEED;p.ydelta=-PENGUINSPEED;
		break;
		case 2:
			p.xdelta=-PENGUINSPEED;p.ydelta= PENGUINSPEED;
		break;
		case 3:
			p.xdelta=-PENGUINSPEED;p.ydelta=-PENGUINSPEED;
		break;
		default: // this won't happen, but it shuts up the warning.
			p.xdelta=-PENGUINSPEED;p.ydelta=-PENGUINSPEED;
		break;
	}
	
	// For debugging only, of course -- this makes crippled penguins.
	//p.xdelta=0; p.ydelta=0;
	
	p.speedslower=0;
	p.x=x;
	p.y=y;

	return p;
}

void penguin_draw(Penguin* p)
{
	C2D_DrawImageAt(theme.spriteimage, p->x, p->y, 0.3f, NULL, SPRITE_SIZE, SPRITE_SIZE);
}

PenguinHit penguin_move(Penguin* p)
{
	bool hitline1=false;
	bool hitline2=false;
	int newx, newy;
	int checkx,checky;
	int movex=0,movey=0;

	switch (options_difficulty_current())
	{
		case NORMAL:
			if (p->speedslower)
				{ movex=p->xdelta/2; movey=p->ydelta/2; }
			else
				{ movex=p->xdelta; movey=p->ydelta; }
			p->speedslower=!p->speedslower;
		break;
		case HARD:
			movex=p->xdelta; movey=p->ydelta;
		break;
		case EASY:
			movex=p->xdelta/2; movey=p->ydelta/2;
		break;
	}
		
	newx=p->x+movex;
	newy=p->y+movey;

	
	grid_mark(p->x,p->y,BLOCKWIDTH,BLOCKHEIGHT,' ');
	
	if (movex>0) checkx = newx+BLOCKWIDTH;
	else checkx = newx;

	if (grid_at(checkx,p->y)==' ' && grid_at(checkx,p->y+BLOCKHEIGHT-1)==' ')
	{
		p->x+=movex;
	}
	else if (grid_at(checkx,p->y)=='1' || grid_at(checkx,p->y+BLOCKHEIGHT-1)=='1')
	{
		hitline1=true;
		p->x+=movex;	
	}
	else if (grid_at(checkx,p->y)=='2' || grid_at(checkx,p->y+BLOCKHEIGHT-1)=='2')
	{
		hitline2=true;
		p->x+=movex;	
	}
	else if ((grid_at(checkx,p->y)=='w' || grid_at(checkx,p->y)==' ' ) 
	      && (grid_at(checkx,p->y+BLOCKHEIGHT-1)=='w' || grid_at(checkx,p->y+BLOCKHEIGHT-1)==' '))
	{
		// this is used in the intro. maybe some place else too in the future.
		// should it be merged into the first line above? maybe.
		p->x+=movex;
	}
	else
	{
		p->xdelta=-p->xdelta;
	}

	if (movey>0) checky = newy+BLOCKHEIGHT;
	else checky = newy;
	
	if (grid_at(p->x,checky)==' ' && grid_at(p->x+BLOCKWIDTH-1,checky)==' ')
	{
		p->y+=movey;
	}
	else if (grid_at(p->x,checky)=='1' || grid_at(p->x+BLOCKWIDTH-1,checky)=='1')
	{
		//printf("Hit 1\n");
		hitline1=true;
		p->y+=movey;		
	}
	else if (grid_at(p->x,checky)=='2' || grid_at(p->x+BLOCKWIDTH-1,checky)=='2')
	{
		//printf("Hit 2\n");
		hitline2=true;
		p->y+=movey;		
	}
	else if ((grid_at(p->x,checky)=='w' || grid_at(p->x,checky)==' ') 
	      && (grid_at(p->x+BLOCKWIDTH-1,checky)=='w' || grid_at(p->x+BLOCKWIDTH-1,checky)==' '))
	{
		// this is used in the intro. maybe some place else too in the future.
		// should it be merged into the first line above? maybe.
		p->y+=movey;
	}

	else
	{
		p->ydelta=-p->ydelta;
	}

	grid_mark(p->x,p->y,BLOCKWIDTH,BLOCKHEIGHT,'*');

	if (hitline1 && hitline2) return BOTH;
	if (hitline1) return LINE1;
	if (hitline2) return LINE2;
	return NONE;
}
