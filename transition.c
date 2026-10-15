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
#include "globals.h"
#include "line.h"
#include "penguin.h"
#include "level.h"
#include "text.h"
#include "cursor.h"
#include "laundry.h"
#include "hiscore.h"
#include "dialog.h"
#include "themes.h"
#include "event.h"

// FIX -- it'd be better to draw text once and then scroll that graphic,
// of course.

static int transition_scrolltext(char * firsttext, SDL_Rect* firstrect, Uint32 firstcolor, char * secondtext, SDL_Rect* secondrect, Uint32 secondcolor, int doneonfinish);

int transition_intermission(ScoreSheet * levelscore, int nextlevel)
{
	int quit=false;
	SDL_Event event;
	SDL_Rect scorerect, bonusrect;
	char scoretext[30];
	char bonustext[30];

	snprintf(scoretext,30,"SCORE: %d",levelscore->basescore);
	snprintf(bonustext,30, "BONUS: %d",levelscore->clearbonus + levelscore->lifebonus);

	// FIX -- play some truimphant but not annoying sound
	
	// clear any pending events
	SDL_Delay(10); // needed? probably not.
	while (event_poll(&event)) if (event.type == SDL_QUIT) { quit=true; }

	cursor_set(CURSORCLICK);

	scorerect.h=CHARHEIGHT*4;
	scorerect.w=text_getwidth(4,scoretext);
	
	bonusrect.h=CHARHEIGHT*4;
	bonusrect.w=text_getwidth(4,bonustext);

	if (scorerect.w>bonusrect.w)
		scorerect.x=(screen->w - scorerect.w) / 2;
	else
		scorerect.x=(screen->w - bonusrect.w) / 2;
	scorerect.y=HEIGHT-CHARHEIGHT*5; // extra space for separation

	bonusrect.x=scorerect.x;
	bonusrect.y=HEIGHT-CHARHEIGHT*4;
	
	// wait for click, scroll score
	quit=transition_scrolltext(scoretext, &scorerect, color.scorescrolltext, bonustext, &bonusrect, color.bonusscrolltext, false);

	// and clear any more events, for good luck.
	while (event_poll(&event)) if (event.type == SDL_QUIT) quit=true;
			
	//printf("Level %d completed. ",level);	
	return(quit);
}

int transition_gameover(long finalscore)
{
	int quit=false;
	SDL_Event event;
	SDL_Rect loserrect, finalrect;

	char finaltext[30];

	snprintf(finaltext,30,"FINAL SCORE: %ld",finalscore);
	

	loserrect.h=CHARHEIGHT*4;
	loserrect.w=text_getwidth(4,"GAME OVER");
	loserrect.x=(screen->w - loserrect.w) / 2;
	loserrect.y=HEIGHT-CHARHEIGHT*4;

	finalrect.h=CHARHEIGHT*4;
	finalrect.w=text_getwidth(4,finaltext);
	finalrect.x=(screen->w - finalrect.w) / 2;
	finalrect.y=HEIGHT-CHARHEIGHT*4;

	// clear any pending events
	SDL_Delay(10); // needed? probably not.
	while (event_poll(&event)) if (event.type == SDL_QUIT) { quit=true; }

	if (!hiscore_check(finalscore))
		cursor_set(CURSORCLICK);
	else 
		cursor_set(CURSORARROW);


	// wait for click, scroll score
	quit=transition_scrolltext("GAME OVER", &loserrect, color.gameovertext, finaltext, &finalrect,  color.scorescrolltext, false);
		
	if (hiscore_check(finalscore))
	{
		// FIX -- play some truimphant but not annoying sound 

		if (dialog_gethighusername(finalscore>hiscoreval[0])==POPUPQUITGAME)
			quit=true;
		
		if(!hiscore_add(username,finalscore,true))
		{ // I don't think we need to say this; people can figure it out by looking at the numbers
		  // but don't comment out the addhighscore function call above, of course. :)
			// fprintf(stderr,"Ouch: looks like someone beat your score while you were typing your name!\n");
		}

		// jump text to top
		loserrect.y=(HEIGHT/2-31);
		text_put(loserrect.x,loserrect.y,4,color.gameovertext,"GAME OVER");			
		laundry_soil(loserrect);
		finalrect.y=(HEIGHT/2);
		text_put(finalrect.x,finalrect.y,4,color.scorescrolltext,finaltext);
		laundry_soil(loserrect);
		laundry_clean();
	}

			
	// and clear any more events, for good luck.
	while (event_poll(&event)) if (event.type == SDL_QUIT) quit=true;
			
	//printf("Game over.\n");	
	return(quit);
}


int transition_scrolltext(char * firsttext, SDL_Rect* firstrect, Uint32 firstcolor, char * secondtext, SDL_Rect* secondrect, Uint32 secondcolor, int doneonfinish)
{
	int quit=false;
	int done=false;
	SDL_Event event;
	
	SDL_Surface * bgsave=SDL_CreateRGBSurface(SDL_SWSURFACE,WIDTH,HEIGHT,screen->format->BitsPerPixel,0,0,0,0);

	// get background
	SDL_BlitSurface(screen, NULL, bgsave, NULL);
	

	// wait for click, scroll score (or whatever)
	while (!done) 
	{
		while(event_poll(&event));

		if (event.type == SDL_QUIT)
		{
			done=true; quit=true;
		}
		else if (event.type==SDL_MOUSEBUTTONDOWN)
		{
			if (event.button.button==1)
			{
					done=true;
			}
		}
		else if (event.type == SDL_KEYUP)
		{
			switch(translatekeyevent(&event))
			{
				case KEYCANCEL:     // falls through
				case KEYMENU:       // falls through
				case KEYSWITCHLINE: // falls through
				case KEYSTARTLINE:  
					done=true;
				default:
				break;
			}
		}
		
		if (firstrect->y>(HEIGHT/2-30))
		{
			firstrect->y--; 
			text_put(firstrect->x,firstrect->y,4,firstcolor,firsttext);
			laundry_soil(*firstrect);
			laundry_clean();		
			SDL_BlitSurface(bgsave,firstrect, screen, firstrect);
		}
		else if (firstrect->y==(HEIGHT/2-30))
		{
			// un-laundry_clean. :)
			firstrect->y--; 
			text_put(firstrect->x,firstrect->y,4,firstcolor,firsttext);
			laundry_soil(*firstrect);
			laundry_clean();
		}
		else if (secondrect->y>(HEIGHT/2))
		{
			secondrect->y--;
			text_put(secondrect->x,secondrect->y,4,secondcolor,secondtext);
			laundry_soil(*secondrect);
			laundry_clean();
			SDL_BlitSurface(bgsave,secondrect, screen, secondrect);
		}
		else if (secondrect->y==(HEIGHT/2))
		{
			// un-laundry_clean. :)
			secondrect->y--;
			text_put(secondrect->x,secondrect->y,4,secondcolor,secondtext);
			laundry_soil(*secondrect);
			laundry_clean();
		}
		else
		{
			if (doneonfinish) done=true;
			laundry_clean();
		}		
		
		SDL_Delay(10);
	}
	SDL_FreeSurface(bgsave);
	return(quit);
}
