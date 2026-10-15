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

#include <SDL.h>
#include <stdio.h>
#include <string.h>
#include "icebreaker.h"
#include "globals.h"
#include "text.h"
#include "laundry.h"
#include "dialog.h"
#include "menu.h"
#include "sound.h"
#include "hiscore.h"
#include "options.h"
#include "fullscreen.h"
#include "themes.h"
#include "grid.h"
#include "status.h"
#include "level.h"
#include "event.h"

// fix -- this file is too big; we should split the menu stuff out
// and put it in menu.c

static PopupReturnType menu_handler(SDL_Rect menurect, int menulength, char ** menuitems, PopupReturnType (**menufunctions)(char *, int), int menuvaluetextwidth );
static void menu_draw(SDL_Rect menurect, int menulength, char ** menuitems, PopupReturnType (**menufunctions)(char *, int), int menuvaluetextwidth, int highlighted);

static PopupReturnType menu_item_newgame(char * notused, int alsonotused);
static PopupReturnType menu_item_options(char * notused, int alsonotused);
static PopupReturnType menu_item_highscores(char * notused, int alsonotused);
static PopupReturnType menu_item_help(char * notused, int alsonotused);
static PopupReturnType menu_item_quit(char * notused, int alsonotused);

static PopupReturnType menu_item_sound(char * val, int mbutton);
static PopupReturnType menu_item_autopause(char * val, int mbutton);
static PopupReturnType menu_item_fullscreen(char * val, int mbutton);
static PopupReturnType menu_item_difficulty(char * val, int mbutton);
static PopupReturnType menu_item_theme(char * val, int mbutton);


// puting this here is ugly, but it's less ugly than many of the
// alternatives -- it's very hard to switch themes on the fly
static SDL_Surface * optionsmenusave;

#define MAXMENUITEMS 10
#define MAXMENUVALUELENGTH 10


#define MAINMENULENGTH 5
PopupReturnType menu_popuplevel()
{
	SDL_Rect menurect, menubuttonrect;
	PopupReturnType rc;
	SDL_Surface * levelmenusave;

	char * mainmenu[MAINMENULENGTH] = { "NEW GAME", "OPTIONS", "HIGH SCORES", "HELP","QUIT" };
	PopupReturnType (*mainmenufunctions[MAINMENULENGTH])(char *,int) = { &menu_item_newgame, &menu_item_options, &menu_item_highscores, &menu_item_help, &menu_item_quit };
	
	menurect.w=128;
	menurect.h=7+(MAINMENULENGTH*(CHARHEIGHT*2+3));
	menurect.x=BORDERRIGHT-menurect.w+5;
	menurect.y=BORDERBOTTOM-menurect.h+9;
	
	menubuttonrect.x=WIDTH-(CHARWIDTH*2*4)-MARGINRIGHT-4;
	menubuttonrect.y=BOTTOMSTATUSY;
	menubuttonrect.w=CHARWIDTH*2*4+3;
	menubuttonrect.h=CHARHEIGHT*2+3;

	levelmenusave = SDL_CreateRGBSurface(SDL_SWSURFACE,menurect.w,menurect.h,screen->format->BitsPerPixel,0,0,0,0);
	SDL_BlitSurface(screen, &menurect, levelmenusave, NULL);

	rc=menu_handler(menurect, MAINMENULENGTH, mainmenu, mainmenufunctions, 0);


	if (rc != POPUPQUITGAME)
	{
		if (gameflags.themechanged)
		{
			if (!strcmp(options.theme,"random") && rc != POPUPNEWGAME)
				themes_settheme("random");
	
			level_redrawwhole();
			menu_drawbutton(&menubuttonrect,true);
			gameflags.themechanged=false;
			laundry_updateall();
		}
		else
		{
			SDL_BlitSurface(levelmenusave, NULL, screen, &menurect);
			laundry_soil(menurect);
		}
	}	
	
	SDL_FreeSurface(levelmenusave);	
	
	return rc;
}

PopupReturnType menu_item_newgame(char * notused, int alsonotused)
{
	return POPUPNEWGAME;
}


PopupReturnType menu_item_options(char * notused, int alsonotused)
{
	SDL_Event event;
	int x, y;
	
	PopupReturnType rc=menu_popupoptions();

	if (rc!=POPUPEXITMENU)
	{
		return rc;
	}
	else
	{
		// add a fake event so "Options" gets un-highlighted if need be.
		SDL_GetMouseState(&x,&y);
		event.motion.x = x;
		event.motion.y = y;
		event.type = SDL_MOUSEMOTION;
		SDL_PushEvent(&event);
		return POPUPDONOTHING;
	}
}


PopupReturnType menu_item_highscores(char * notused, int alsonotused)
{
	PopupReturnType rc;
	SDL_Event event;
	rc=dialog_popuphighscores();
	int x, y;
	// add a fake event so menuitem gets un-highlighted if need be.
	SDL_GetMouseState(&x,&y);
	event.motion.x = x;
	event.motion.y = y;
	event.type = SDL_MOUSEMOTION;
	SDL_PushEvent(&event);
	return rc;
}


PopupReturnType menu_item_help(char * notused, int alsonotused)
{
	PopupReturnType rc;
	SDL_Event event;
	rc=dialog_popuphelp();
	int x, y;
	// add a fake event so menuitem gets un-highlighted if need be.
	SDL_GetMouseState(&x,&y);
	event.motion.x = x;
	event.motion.y = y;
	event.type = SDL_MOUSEMOTION;
	SDL_PushEvent(&event);

	return rc;
}


PopupReturnType menu_item_quit(char * notused, int alsonotused)
{
	return POPUPQUITGAME;
}

#define OPTIONSMENULENGTH 5

PopupReturnType menu_popupoptions()
{
	SDL_Rect menurect;
	GameDifficultyType originaldifficulty=options.difficulty;
	char originaltheme[MAXTHEMENAMELENGTH+11];
	PopupReturnType rc;
	int originalthemecl=false;
	int recallme=false;


	char * optionsmenu[OPTIONSMENULENGTH] = { "SOUND", "AUTO PAUSE", "FULL SCREEN", "DIFFICULTY", "THEME" };
	PopupReturnType (*optionsmenufunctions[OPTIONSMENULENGTH])(char *,int) = { &menu_item_sound, &menu_item_autopause, &menu_item_fullscreen, &menu_item_difficulty, &menu_item_theme };

// truncation is on purpose
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wformat-truncation"
	if (strlen(commandline.theme)>0)
	{
		snprintf(originaltheme,MAXTHEMENAMELENGTH+1,"%s",commandline.theme);
		originalthemecl=true;
	}
	else
	{
		snprintf(originaltheme,MAXTHEMENAMELENGTH+1,"%s",options.theme);
		originalthemecl=false;	
	}
#pragma GCC diagnostic pop	
				
	menurect.w=229;
	menurect.h=7+(OPTIONSMENULENGTH*(CHARHEIGHT*2+3));
	menurect.x=BORDERRIGHT-menurect.w+10;
	menurect.y=BORDERBOTTOM-menurect.h+9-((CHARHEIGHT*2+4)*4)-2;

	optionsmenusave = SDL_CreateRGBSurface(SDL_SWSURFACE,menurect.w,menurect.h,screen->format->BitsPerPixel,0,0,0,0);
	SDL_BlitSurface(screen, &menurect, optionsmenusave, NULL);

	rc=menu_handler(menurect, OPTIONSMENULENGTH, optionsmenu, optionsmenufunctions, 84);

	if (rc==POPUPEXITMENU)
	{
		if ((originalthemecl && strcmp(commandline.theme, originaltheme)) ||
		    (!originalthemecl && strcmp(options.theme, originaltheme)) )
		{
			gameflags.themechanged=true;
		}
	}

	if (rc==POPUPEXITMENU && options.difficulty != originaldifficulty)
	{
		if (level_checkifatstart())
		{
			rc=POPUPNEWGAME;
		}
		else
		{
			switch (dialog_yesno("Changing difficulty","requires restart.","End this game?"))
			{
				case POPUPQUITGAME:
					rc=POPUPQUITGAME;
				break;
				case POPUPYES:
					rc=POPUPNEWGAME;
				break;
				case POPUPNO:
					options.difficulty=originaldifficulty; // put things back. :)
					recallme=true;
				break;
				default:
					fprintf(stderr,"Internal weirdness: yes/no dialog box returned something other than yes or no.\n"
					               "That shouldn't ever happen.\n");
				break;
			}
		}
	}

	//options_write(); // probably no need to do this until exit
	
	// and restore the background
	if (rc != POPUPQUITGAME)
	{
		SDL_BlitSurface(optionsmenusave, NULL, screen, &menurect);
		laundry_soil(menurect);
	}

	SDL_FreeSurface(optionsmenusave);	
	

	if (recallme) rc=menu_popupoptions(); // this is a bit sick.	
	return rc;
}


PopupReturnType menu_item_sound(char * val, int mbutton)
{
	if (strlen(val)==0)
	{
		if (gameflags.soundsystemworks)
		{
			if (options.sound==SOUNDON)
				strncpy(val,"on",MAXMENUVALUELENGTH);
			else
				strncpy(val,"off",MAXMENUVALUELENGTH);
		}
		else
		{
			strncpy(val,"n/a",MAXMENUVALUELENGTH);
		}
		return POPUPDONOTHING;
	}
	else
	{
		if (!gameflags.soundsystemworks) return POPUPDONOTHING;
		if (options.sound==SOUNDON)
		{
			options.sound=SOUNDOFF;
			strncpy(val,"off",MAXMENUVALUELENGTH);
		}
		else
		{
			options.sound=SOUNDON;	
			strncpy(val,"on",MAXMENUVALUELENGTH);
		}
		return POPUPREDRAWME;
	}
}

PopupReturnType menu_item_autopause(char * val, int mbutton)
{
	if (strlen(val)==0)
	{
		if (options.autopause==AUTOPAUSEON)
			strncpy(val,"on",MAXMENUVALUELENGTH);
		else
			strncpy(val,"off",MAXMENUVALUELENGTH);
		return POPUPDONOTHING;
	}
	else
	{
		if (options.autopause==AUTOPAUSEON)
		{
			options.autopause=AUTOPAUSEOFF;
			strncpy(val,"off",MAXMENUVALUELENGTH);
		}
		else
		{
			options.autopause=AUTOPAUSEON;	
			strncpy(val,"on",MAXMENUVALUELENGTH);
		}
		return POPUPREDRAWME;
	}
}

PopupReturnType menu_item_fullscreen(char * val, int mbutton)
{
	// fix -- cope with failure to switch
	if (strlen(val)==0)
	{
		switch (options.fullscreen)
		{
			case FULLSCREENOFF:
				strncpy(val,"off",MAXMENUVALUELENGTH);
			break;
			case FULLSCREENON:
				strncpy(val,"on",MAXMENUVALUELENGTH);
			break;
			case FULLSCREENALWAYS:
				strncpy(val,"always",MAXMENUVALUELENGTH);
			break;
			case FULLSCREENUNKNOWN: // this should never happen!
				strncpy(val,"???",MAXMENUVALUELENGTH);
			break;
		}
		return POPUPDONOTHING;
	}
	else
	{
		if (mbutton==1 || mbutton==4) // left or scroll forwards
		{
			switch (options.fullscreen)
			{
				case FULLSCREENOFF:
					options.fullscreen=FULLSCREENON;
					strncpy(val,"on",MAXMENUVALUELENGTH);
					fullscreen_make();
				break;
				case FULLSCREENON:
					options.fullscreen=FULLSCREENALWAYS;
					strncpy(val,"always",MAXMENUVALUELENGTH);
				break;
				case FULLSCREENALWAYS:
					options.fullscreen=FULLSCREENOFF;
					strncpy(val,"off",MAXMENUVALUELENGTH);
					fullscreen_makewindowed();
				break;
				case FULLSCREENUNKNOWN: // this should never happen!
					options.fullscreen=FULLSCREENOFF;
					strncpy(val,"off",MAXMENUVALUELENGTH);
					fullscreen_makewindowed();
				break;
			}
		}
		else // right or middle or scroll back
		{
			switch (options.fullscreen)
			{
				case FULLSCREENOFF:
					options.fullscreen=FULLSCREENALWAYS;
					strncpy(val,"always",MAXMENUVALUELENGTH);
					fullscreen_make();
				break;
				case FULLSCREENON:
					options.fullscreen=FULLSCREENOFF;
					strncpy(val,"off",MAXMENUVALUELENGTH);
					fullscreen_makewindowed();
				break;
				case FULLSCREENALWAYS:
					options.fullscreen=FULLSCREENON;
					strncpy(val,"on",MAXMENUVALUELENGTH);
				break;
				case FULLSCREENUNKNOWN: // this should never happen!
					options.fullscreen=FULLSCREENOFF;
					strncpy(val,"off",MAXMENUVALUELENGTH);
					fullscreen_makewindowed();
				break;
			}
		}
		return POPUPREDRAWME;
	}
}


PopupReturnType menu_item_difficulty(char * val, int mbutton)
{
	if (strlen(val)==0)
	{
		switch (options.difficulty)
		{
			case NORMAL:
				strncpy(val,"normal",MAXMENUVALUELENGTH);
			break;
			case EASY:
				strncpy(val,"easy",MAXMENUVALUELENGTH);
			break;
			case HARD:
				strncpy(val,"hard",MAXMENUVALUELENGTH);
			break;
		}
		return POPUPDONOTHING;
	}
	else
	{
		if (mbutton==1 || mbutton==4) // left or scroll forwards
		{
			switch (options.difficulty)
			{
				case NORMAL:
					options.difficulty=EASY;
					strncpy(val,"easy",MAXMENUVALUELENGTH);
				break;
				case EASY:
					options.difficulty=HARD;
					strncpy(val,"hard",MAXMENUVALUELENGTH);
				break;
				case HARD:
					options.difficulty=NORMAL;
					strncpy(val,"normal",MAXMENUVALUELENGTH);
				break;
			}
		}
		else // right or middle or scroll back
		{
			switch (options.difficulty)
			{
				case NORMAL:
					options.difficulty=HARD;
					strncpy(val,"hard",MAXMENUVALUELENGTH);
				break;
				case EASY:
					options.difficulty=NORMAL;
					strncpy(val,"normal",MAXMENUVALUELENGTH);
				break;
				case HARD:
					options.difficulty=EASY;
					strncpy(val,"easy",MAXMENUVALUELENGTH);
				break;
			}
		}
		return POPUPREDRAWME;
	}
}

PopupReturnType menu_item_theme(char * val, int mbutton)
{
	char** themelist;
	int t;
	int themecount;
	SDL_Rect tmprect;
	
	// FIX -- this is kind of kludgy -- why define this twice?
	char * mainmenu[MAINMENULENGTH] = { "NEW GAME", "OPTIONS", "HIGH SCORES", "HELP","QUIT" };	

	// fix -- all of these are also stupidly duplicated
	SDL_Rect mainmenurect;
	SDL_Rect optionsmenurect;
	SDL_Rect menubuttonrect;
	mainmenurect.w=128;
	mainmenurect.h=7+(MAINMENULENGTH*(CHARHEIGHT*2+3));
	mainmenurect.x=BORDERRIGHT-mainmenurect.w+5;
	mainmenurect.y=BORDERBOTTOM-mainmenurect.h+9;

	optionsmenurect.w=229;
	optionsmenurect.h=7+(OPTIONSMENULENGTH*(CHARHEIGHT*2+3));
	optionsmenurect.x=BORDERRIGHT-optionsmenurect.w+10;
	optionsmenurect.y=BORDERBOTTOM-optionsmenurect.h+9-((CHARHEIGHT*2+4)*4)-2;

	menubuttonrect.x=WIDTH-(CHARWIDTH*2*4)-MARGINRIGHT-4;
	menubuttonrect.y=BOTTOMSTATUSY;
	menubuttonrect.w=CHARWIDTH*2*4+3;
	menubuttonrect.h=CHARHEIGHT*2+3;


	if (strlen(val)==0)
	{
	
		
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wstringop-truncation"
		if (strlen(commandline.theme)>0)
			strncpy(val,commandline.theme,MAXMENUVALUELENGTH);
		else
			strncpy(val,options.theme,MAXMENUVALUELENGTH);
		return POPUPDONOTHING;
#pragma GCC diagnostic pop		
	}
	else
	{
		themecount=themes_getthemenames(&themelist);
		
		if (strlen(commandline.theme)>0)
		{
			t=themes_getthemenumber(themelist,themecount,commandline.theme);
			// once we've changed the theme, there's no going back
			*(commandline.theme)='\0'; // makes this ""
		}
		else
		{
			t=themes_getthemenumber(themelist,themecount,options.theme);
		}

		if (((mbutton==1 || mbutton==4) && t==themecount-1) || ((mbutton!=1 && mbutton!=4) && t==0))
		{ // "random", at the end/beginning of the list
			
			strncpy(val,"random",MAXMENUVALUELENGTH);
			snprintf(options.theme,MAXMENUVALUELENGTH,"random");
			
			themes_settheme("linux"); // just for pretty
			
			level_redrawwhole();
			menu_draw(mainmenurect, MAINMENULENGTH, mainmenu, NULL, 0, 1);
			menu_drawbutton(&menubuttonrect,true);
			
			text_put((WIDTH-text_getwidth(10,"?"))/2,(HEIGHT-CHARHEIGHT*10)/2,10,color.gameovertext,"?");
			gameflags.themechanged=true;
		}
		else
		{
			if (mbutton==1 || mbutton==4) // left click or scroll forwards
			{
				strncpy(val,themelist[(t+1)%themecount],MAXMENUVALUELENGTH);
				snprintf(options.theme,MAXMENUVALUELENGTH,"%s",themelist[(t+1)%themecount]);
			}
			else // right or middle or scroll back
			{
				if (t<0) t=themecount;
				strncpy(val,themelist[(t-1)%themecount],MAXMENUVALUELENGTH);
				snprintf(options.theme,MAXMENUVALUELENGTH,"%s",themelist[(t-1)%themecount]);
			}
			
			themes_settheme(options.theme);
			
			level_redrawwhole();
			menu_draw(mainmenurect, MAINMENULENGTH, mainmenu, NULL, 0, 1);
			menu_drawbutton(&menubuttonrect,true);
		}
		SDL_BlitSurface(screen, &optionsmenurect, optionsmenusave, NULL);
		
		themes_freethemenames(&themelist,themecount);
		
		tmprect.w=WIDTH; tmprect.h=HEIGHT;
		tmprect.x=0;tmprect.y=0;
		laundry_soil(tmprect);
		
		
		return POPUPDONOTHING;
	}
}



/* displays a menu; takes a rect for position and size, the length of the
 * menu, an array of menu items, an array of pointers to functions for
 * each menu item (functions should return type MenuReturnType
 * indicating what to do when the item is clicked) and a flag telling whether
 * menu items are option/value pairs.
 */
PopupReturnType menu_handler(SDL_Rect menurect, int menulength, char ** menuitems, PopupReturnType (**menufunctions)(char *, int), int menuvaluetextwidth)
{
	SDL_Rect insiderect;
	SDL_Rect menuitemrect[MAXMENUITEMS];
	char menuvalues[MAXMENUITEMS][MAXMENUVALUELENGTH];
	int menuitemglow=-1;

	int menuitempressed=false;
	SDL_Event event;
	PopupReturnType rc=POPUPDONOTHING;
	int redrawmenuflag=false;
	int domenuitemflag=false;
	Uint8 domenuitembutton=0;

	int i;
	int tmpx, tmpy;

	SDL_EnableKeyRepeat(500,90);
	
	insiderect.w=menurect.w-2;
	insiderect.h=menurect.h-2;
	insiderect.x=menurect.x+1;
	insiderect.y=menurect.y+1;

	// fix -- use menu_draw() here?
	SDL_FillRect(screen,&menurect,color.gridline);
	SDL_FillRect(screen,&insiderect,color.background);

	// find current option values
	for (i=0;i<menulength;i++)
	{
		menuitemrect[i].x=insiderect.x;
		menuitemrect[i].y=insiderect.y+2+(i*(CHARHEIGHT*2+3));
		menuitemrect[i].w=insiderect.w;
		menuitemrect[i].h=(CHARHEIGHT*2)+3;
		text_put(menuitemrect[i].x+5,menuitemrect[i].y+3,2,color.normaltext,menuitems[i]);

		strncpy(menuvalues[i],"",MAXMENUVALUELENGTH);
		if (menuvaluetextwidth)
		{
			if (menufunctions!=NULL && menufunctions[i]!=NULL)
				(*menufunctions[i])(menuvalues[i],0);
			text_put(menuitemrect[i].x+menuitemrect[i].w-menuvaluetextwidth,menuitemrect[i].y+3,2,color.normaltext,menuvalues[i]);
		}
	}		
	laundry_soil(menurect);	

	laundry_clean(); // fix -- any point in not doing this first thing in the
	        // loop and removing the laundry_clean at the bottom of the loop?
	
	do
	{
		SDL_WaitEvent(NULL); // no new CPU cooler needed. :)
		while (event_poll(&event))
		{
			if (event.type == SDL_QUIT)
			{
				rc=POPUPQUITGAME;
			}
			else if (event.type == SDL_MOUSEBUTTONDOWN)
			{ 
				if (event.button.x>menurect.x &&
				    event.button.y>menurect.y &&
				    event.button.x<menurect.x + menurect.w &&
				    event.button.y<menurect.y + menurect.h)
				{
					if (event.button.button == 1 || menuvaluetextwidth) // we only care about right clicks on items with options
					{
						if (event.button.y-menurect.y>=3)
						if (menuitemglow!=(event.button.y-(menurect.y+3))/(CHARHEIGHT*2+3))
						{
							menuitemglow=(event.button.y-(menurect.y+3))/(CHARHEIGHT*2+3);
							
							redrawmenuflag=true;
						}
						if (menuitemglow>=menulength)
							menuitemglow=-1;
						else
							menuitempressed=true;
					}
				}
				else
				{
					rc=POPUPEXITMENU;
				}
			}
			else if (event.type == SDL_MOUSEBUTTONUP)
			{
				if (menuitempressed && (event.button.button == 1 || menuvaluetextwidth)) // again, we only care about right clicks on items with options
				{
					// in area, button was down
					if (event.motion.x>menurect.x &&
					    event.motion.y>(menurect.y + 2) &&
					    event.motion.x<menurect.x + menurect.w &&
					    event.motion.y<menurect.y + menurect.h - 2)
					{
						if (menufunctions!=NULL && menufunctions[menuitemglow]!=NULL)
						{	
							domenuitemflag=true;
							domenuitembutton=event.button.button;
						}
					}					
					menuitempressed=false;
				}
			}
			else if (event.type == SDL_MOUSEMOTION)
			{
				// are we in the menu area?
				if (event.motion.x>menurect.x &&
				     event.motion.y>(menurect.y + 2) &&
				     event.motion.x<menurect.x + menurect.w &&
				     event.motion.y<menurect.y + menurect.h - 2)
				{
					menuitemglow=(event.motion.y-(menurect.y+3))/(CHARHEIGHT*2+3);
					if (menuitemglow>=menulength) menuitemglow=-1; 
					redrawmenuflag=true;
				}
				else
				{
					if (menuitemglow != -1 && !menuitempressed)
					{
						menuitemglow=-1;
						redrawmenuflag=true;
					}
				}
			
			}
			else if (event.type==SDL_KEYDOWN)
			{
				switch(translatekeyevent(&event))
				{
					case KEYMENU: // falls through
					case KEYCANCEL:
						rc=POPUPEXITMENU;
					case KEYMOVEUP:
						if (!menuitempressed)
						{
							menuitemglow--;
							if (menuitemglow<0) menuitemglow=menulength-1;
							redrawmenuflag=true;
						}
					break;
					case KEYMOVEDOWN:
						if (!menuitempressed)
						{
							menuitemglow++;
							if (menuitemglow>=menulength) menuitemglow=0;
							redrawmenuflag=true;
						}
					break;
					case KEYSTARTLINE:
						if (menuitemglow!=-1 && menufunctions!=NULL && menufunctions[menuitemglow]!=NULL)
						{	
							domenuitemflag=true;
							domenuitembutton=1;
						}
					break;
					case KEYSWITCHLINE:
						if (menuvaluetextwidth && menuitemglow!=-1 && menufunctions!=NULL && menufunctions[menuitemglow]!=NULL)
						{	
							domenuitemflag=true;
							domenuitembutton=2;
						}
					break;
					case KEYMOVERIGHT:
						if (menuvaluetextwidth && menuitemglow!=-1 && menufunctions!=NULL && menufunctions[menuitemglow]!=NULL)
						{	
							domenuitemflag=true;
							domenuitembutton=1;
						}
					break;
					case KEYMOVELEFT:
						if (menuvaluetextwidth && menuitemglow!=-1 && menufunctions!=NULL && menufunctions[menuitemglow]!=NULL)
						{	
							domenuitemflag=true;
							domenuitembutton=2;
						}
					break;
					default:
					break;
				}

			}
		}
		
		if (domenuitemflag)
		{
			// this calls the menu functions, in case that's not obvious
			rc=(*menufunctions[menuitemglow])(menuvalues[menuitemglow],domenuitembutton); 
			domenuitemflag=false;
			redrawmenuflag=true;
		}
						
		
		if (rc==POPUPREDRAWME)
		{	
			rc=POPUPDONOTHING;
			redrawmenuflag=true;
		}
	
		
		if (redrawmenuflag)
		{
			SDL_FillRect(screen,&menurect,color.gridline);
			SDL_FillRect(screen,&insiderect,color.background);
			for (i=0;i<menulength;i++)
			{
					if (i == menuitemglow)
				{
					SDL_FillRect(screen,&menuitemrect[i],color.menuhighlight);
					text_put(menuitemrect[i].x+5,menuitemrect[i].y+3,2,color.background,menuitems[i]);
					if (menuvaluetextwidth) text_put(menuitemrect[i].x+menuitemrect[i].w-menuvaluetextwidth,menuitemrect[i].y+3,2,color.background,menuvalues[i]);
				}
				else
				{
					text_put(menuitemrect[i].x+5,menuitemrect[i].y+3,2,color.normaltext,menuitems[i]);
					if (menuvaluetextwidth) text_put(menuitemrect[i].x+menuitemrect[i].w-menuvaluetextwidth,menuitemrect[i].y+3,2,color.normaltext,menuvalues[i]);
				}
			}		
			laundry_soil(menurect);
			redrawmenuflag=false;
		}

		laundry_clean();
	} while (rc==POPUPDONOTHING);

	// a fake event to update things that depend on mouse position
	SDL_GetMouseState(&tmpx,&tmpy);
	event.motion.x = tmpx;
	event.motion.y = tmpy;
	event.type = SDL_MOUSEMOTION;
	SDL_PushEvent(&event);

	SDL_EnableKeyRepeat(0,0);
	return(rc);
}


void menu_draw(SDL_Rect menurect, int menulength, char ** menuitems, PopupReturnType (**menufunctions)(char *, int), int menuvaluetextwidth, int highlighted)
{
	SDL_Rect insiderect;
	SDL_Rect menuitemrect[MAXMENUITEMS];
	char menuvalues[MAXMENUITEMS][MAXMENUVALUELENGTH];

	int i;
	
	insiderect.w=menurect.w-2;
	insiderect.h=menurect.h-2;
	insiderect.x=menurect.x+1;
	insiderect.y=menurect.y+1;

	SDL_FillRect(screen,&menurect,color.gridline);
	SDL_FillRect(screen,&insiderect,color.background);

	for (i=0;i<menulength;i++)
	{
		menuitemrect[i].x=insiderect.x;
		menuitemrect[i].y=insiderect.y+2+(i*(CHARHEIGHT*2+3));
		menuitemrect[i].w=insiderect.w;
		menuitemrect[i].h=(CHARHEIGHT*2)+3;

		strncpy(menuvalues[i],"",MAXMENUVALUELENGTH);
		if (i == highlighted)
		{
			SDL_FillRect(screen,&menuitemrect[i],color.menuhighlight);
			text_put(menuitemrect[i].x+5,menuitemrect[i].y+3,2,color.background,menuitems[i]);
			if (menuvaluetextwidth) // get option value
			{
				if (menufunctions!=NULL && menufunctions[i]!=NULL)
					(*menufunctions[i])(menuvalues[i],0);
				text_put(menuitemrect[i].x+menuitemrect[i].w-menuvaluetextwidth,menuitemrect[i].y+3,2,color.background,menuvalues[i]);
			}			
		}
		else
		{
			text_put(menuitemrect[i].x+5,menuitemrect[i].y+3,2,color.normaltext,menuitems[i]);
			if (menuvaluetextwidth)
			{
				if (menufunctions!=NULL && menufunctions[i]!=NULL)
					(*menufunctions[i])(menuvalues[i],0);
				text_put(menuitemrect[i].x+menuitemrect[i].w-menuvaluetextwidth,menuitemrect[i].y+3,2,color.normaltext,menuvalues[i]);
		 	}
		}

	}		
	
	laundry_soil(menurect);	
	
	laundry_clean();
	
}

void menu_drawbutton(SDL_Rect* mbrect,int highlighted)
{
	if (highlighted)
	{
		SDL_FillRect(screen,mbrect,color.menuhighlight);
		text_put(mbrect->x+3,mbrect->y+3,2,color.background,"MENU"); 
	}
	else
	{
		SDL_FillRect(screen,mbrect,color.background);
		text_put(mbrect->x+3,mbrect->y+3,2,color.normaltext,"MENU");
	}
	laundry_soil(*mbrect);
}

