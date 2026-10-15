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
* with this program; if not, write to the Free Software Foundation, Inc.,
* 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
*
*/

#include <SDL.h>

#include "icebreaker.h"
#include "text.h"
#include "globals.h"
#include "hiscore.h"
#include "laundry.h"
#include "status.h"
#include "themes.h"

void status_updatelives(int lives)
{
	SDL_Rect tmprect;
	char tmptext[30]; // should be plenty big.
	
	tmprect.h=CHARHEIGHT*2; tmprect.w=LIVESW;
	tmprect.x=LIVESX; tmprect.y=STATUSY;
	
	SDL_FillRect(screen,&tmprect,color.background);
	
	snprintf(tmptext,30,"LIVES: %d",lives);
	text_put(LIVESX,STATUSY,2,color.normaltext,tmptext);
	
	laundry_soil(tmprect);

}

void status_updatecleared(int clear)
{
	SDL_Rect tmprect;
	char tmptext[30]; // should be plenty big.
	
	tmprect.h=CHARHEIGHT*2; tmprect.w=CLEAREDW;
	tmprect.x=CLEAREDX; tmprect.y=STATUSY;
	
	
	SDL_FillRect(screen,&tmprect,color.background);
	
	snprintf(tmptext,30, "CLEARED: %d%%",clear);
	text_put(CLEAREDX,STATUSY,2,color.normaltext,tmptext);
	
	laundry_soil(tmprect);

}

void status_updatescore(long score)
{
	SDL_Rect tmprect;
	char tmptext[30]; // should be plenty big.
	
	tmprect.h=CHARHEIGHT*2; tmprect.w=SCOREW;
	tmprect.x=SCOREX; tmprect.y=STATUSY;
	
	SDL_FillRect(screen,&tmprect,color.background);
	
	snprintf(tmptext,30, "SCORE: %ld",score);
	text_put(SCOREX,STATUSY,2,color.normaltext,tmptext);

	laundry_soil(tmprect);
}



extern void updatehiscorebox()
{
	SDL_Rect tmprect;
	
	char tmptext[100]; // should be plenty big.
	
	tmprect.x=LIVESX; tmprect.y=BOTTOMSTATUSY;
	tmprect.h=CHARHEIGHT*3;; tmprect.w=WIDTH-(CHARWIDTH*2*4)-MARGINRIGHT-4-LIVESX-2;
	
	
	SDL_FillRect(screen,&tmprect,color.background);
	
	snprintf(tmptext,100,"HIGH SCORE: %ld (%s)",hiscoreval[0],hiscorename[0]);
	text_put(tmprect.x,tmprect.y+3,2,color.normaltext,tmptext);	
	
	laundry_soil(tmprect);
}
