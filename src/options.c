/*
* IceBreaker
* Copyright (c) 2001-2020 Matthew Miller <mattdm@mattdm.org> and
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
#include <ctype.h>
#include <stdlib.h>
#include "icebreaker.h"
#include "globals.h"
#include "icebreaker.h"
#include "options.h"
#include "themes.h"
#include "stdarg.h"

GameOptionsType options;
GameOptionsType commandline;

GameFlagsType gameflags;

static int options_findflag(int argc,char** argv,char* option);
static void options_printversion(void);
static void options_printhelp(void);
static void options_printthemelist(void);

#ifdef WIN32
#  include <windows.h>
typedef struct
{
	HKEY hK;
	DWORD line;
}reg_file;
#  define ICEBREAKER_KEY "Software\\IceBreaker"
#  define OPTFILEHANDLER reg_file
#else
#  define OPTFILEHANDLER FILE
#endif


OPTFILEHANDLER *openoptionfile(char* filename,char* mode);
void options_closeoptionfile(OPTFILEHANDLER *f);
char* readoptionline(char* buffer,int len,OPTFILEHANDLER *f);
int options_writeoptionline(OPTFILEHANDLER *f,char* format,...);

#define FLAGVERSION        "-v"
#define FLAGVERSIONLONG    "--version"
#define FLAGHELP           "-h"
#define FLAGHELPLONG       "--help"
#define FLAGSOUND          "-s"
#define FLAGSOUNDLONG      "--sound"
#define FLAGNOSOUND        "-n"
#define FLAGNOSOUNDLONG    "--nosound"
#define FLAGFULLSCREEN     "-f"
#define FLAGFULLSCREENLONG "--fullscreen"
#define FLAGWINDOWED       "-w"
#define FLAGWINDOWEDLONG   "--windowed"
#define FLAGTHEME          "-t"
#define FLAGTHEMELONG      "--theme"
#define FLAGLISTTHEMES     "-l"
#define FLAGLISTTHEMESLONG "--listthemes"
#define FLAGBENCHMARK      "-b"
#define FLAGBENCHMARKLONG  "--benchmark"

void options_setdefault(void)
{
	options.sound=SOUNDON;
	options.autopause=AUTOPAUSEOFF;	
	options.difficulty=NORMAL;
	options.fullscreen=FULLSCREENOFF;
	snprintf(options.theme,MAXTHEMENAMELENGTH+1,"linux");
}

int options_read(void)
{
	OPTFILEHANDLER * optionfile;
	char linebuf[50];
	char filename[4096]; // fix -- use defined OS constant
	char optbuf[21];
	char valbuf[10+MAXTHEMENAMELENGTH];
	char scanformat[20];
	int i;

	options_setdefault();
	
	snprintf(filename,4096,"%s/%s",homedir,OPTIONFILE);
	
	optionfile=openoptionfile(filename,"r");
	if (optionfile==NULL)
	{
		fprintf(stderr, OPTIONFILE " doesn't exist.\nWelcome to IceBreaker.\n");
		return true;
	}
	
	while(readoptionline(linebuf,50,optionfile))
	{
		for (i=0;i<50;i++)
		{
			if (linebuf[i]=='\0') break;
			linebuf[i]=tolower(linebuf[i]);
		}
		
		sprintf(scanformat,"%%20s %%%ds",10+MAXTHEMENAMELENGTH);	
		if (sscanf(linebuf,"%20s %10s",optbuf,valbuf)==2)
		{
			if (!strcmp(optbuf,"sound"))
			{
				if (!strcmp(valbuf,"on"))
					options.sound=SOUNDON;
				else if (!strcmp(valbuf,"off"))
					options.sound=SOUNDOFF;
			}
			else if (!strcmp(optbuf,"autopause"))
			{
				if (!strcmp(valbuf,"on"))
					options.autopause=AUTOPAUSEON;
				else if (!strcmp(valbuf,"off"))
					options.autopause=AUTOPAUSEOFF;
			}
			else if (!strcmp(optbuf,"fullscreen"))
			{
				if (!strcmp(valbuf,"off"))
					options.fullscreen=FULLSCREENOFF;
				else if (!strcmp(valbuf,"on"))
					options.fullscreen=FULLSCREENON;
				else if (!strcmp(valbuf,"always"))
					options.fullscreen=FULLSCREENALWAYS;
			}
			else if (!strcmp(optbuf,"difficulty"))
			{
				if (!strcmp(valbuf,"normal"))
					options.difficulty=NORMAL;
				else if (!strcmp(valbuf,"hard"))
					options.difficulty=HARD;
				else if (!strcmp(valbuf,"easy"))
					options.difficulty=EASY;
			}
			else if (!strcmp(optbuf,"theme"))
			{
				snprintf(options.theme,MAXTHEMENAMELENGTH+10,"%s",valbuf);
			}
			// FIX: add username
		}
	}
	
	options_closeoptionfile(optionfile);

	return false;
}

int options_write(void)
{
	OPTFILEHANDLER * optionfile;
	char filename[4096];
	snprintf(filename,4096,"%s/%s",homedir,OPTIONFILE);
	
	optionfile=openoptionfile(filename,"w");
	if (optionfile==NULL)
	{
		fprintf(stderr, "Can't write to " OPTIONFILE ".\n");
		return true;
	}
	
	options_writeoptionline(optionfile,"# Icebreaker config file 1.0\n#\n");
	options_writeoptionline(optionfile,"# Separate keywords from values by whitespace. Not case sensitive.\n#\n");
	options_writeoptionline(optionfile,"# %s/" OPTIONFILE " will be overwritten automatically.\n#\n",homedir);
	
	options_writeoptionline(optionfile,"\n# Change this if the crashing noise annoys your neighbors.\n");
	if (options.sound==SOUNDON)
		options_writeoptionline(optionfile,"Sound On\n");
	else if (options.sound==SOUNDOFF)
		options_writeoptionline(optionfile,"Sound Off\n");
		
	options_writeoptionline(optionfile,"\n# AutoPause makes the game pause when the window is out of focus.\n");
	if (options.autopause==AUTOPAUSEON)
		options_writeoptionline(optionfile,"AutoPause On\n");
	else if (options.autopause==AUTOPAUSEOFF)
		options_writeoptionline(optionfile,"AutoPause Off\n");

	options_writeoptionline(optionfile,"\n# Set FullScreen to Always if you want it that way every time.\n");
	options_writeoptionline(optionfile,"# On will use full screen mode once, but then change back to Off.\n");
	if (options.fullscreen==FULLSCREENOFF || options.fullscreen==FULLSCREENON)
		options_writeoptionline(optionfile,"FullScreen Off\n");
	else if (options.fullscreen==FULLSCREENALWAYS)
		options_writeoptionline(optionfile,"FullScreen Always\n");

	options_writeoptionline(optionfile,"\n# Normal is the best way to play. Easy is okay to get started,\n");
	options_writeoptionline(optionfile,"# but you won't get very high scores. Hard is for those who really\n");
	options_writeoptionline(optionfile,"# want a challenge, but scores only slightly higher than normal.\n");
	

	if (options.difficulty==NORMAL)
		options_writeoptionline(optionfile,"Difficulty Normal\n");
	else if (options.difficulty==HARD)
		options_writeoptionline(optionfile,"Difficulty Hard\n");
	else if (options.difficulty==EASY)
		options_writeoptionline(optionfile,"Difficulty Easy\n");


	options_writeoptionline(optionfile,"\n# Themes provide an easy way to select (or tweak!) the appearance\n");
	options_writeoptionline(optionfile,"# and sound of the game. Select from " THEMEFILEEXTENSION " files in the game data\n");
	options_writeoptionline(optionfile,"# directory. Check the docs for info on creating your own themes, too.\n");	

	options_writeoptionline(optionfile,"Theme %s\n",options.theme);

	options_closeoptionfile(optionfile);
	
	return false;
}

int options_parsecommandline(int argc, char** argv)
{
	int i;

	commandline.sound=SOUNDON;
	commandline.autopause=AUTOPAUSEOFF;	
	commandline.difficulty=NORMAL;
	commandline.fullscreen=FULLSCREENUNKNOWN;
	*(commandline.theme)='\0'; // makes this ""

	if ( options_findflag(argc,argv,FLAGVERSION) || options_findflag(argc,argv,FLAGVERSIONLONG) ) 
	{
		options_printversion();
		return 1;
	}

	if ( options_findflag(argc,argv,FLAGHELP) || options_findflag(argc,argv,FLAGHELPLONG) ) 
	{
		options_printhelp();
		return 2;
	}

	if ( options_findflag(argc,argv,FLAGLISTTHEMES) || options_findflag(argc,argv,FLAGLISTTHEMESLONG) ) 
	{
		options_printthemelist();
		return 3;
	}

	if ( options_findflag(argc,argv,FLAGNOSOUND) || options_findflag(argc,argv,FLAGNOSOUNDLONG) ) 
		commandline.sound=SOUNDOFF;

	if ( options_findflag(argc,argv,FLAGBENCHMARK) || options_findflag(argc,argv,FLAGBENCHMARKLONG) ) 
		gameflags.benchmarkmode=true;
	else
		gameflags.benchmarkmode =false;


	if ( options_findflag(argc,argv,FLAGSOUND) || options_findflag(argc,argv,FLAGSOUNDLONG) )
	{
		if (commandline.sound==SOUNDOFF)
		{	
			fprintf(stderr,"You asked for sound to be both on and off. Sorry Schrodinger, we can't do that.\n"
			               "We'll assume \'off\' is what you really meant.\n");
		}
		else
		{
			commandline.sound=SOUNDON;  // redundant. but more clear. :)
		}
	}


	if ( options_findflag(argc,argv,FLAGFULLSCREEN) || options_findflag(argc,argv,FLAGFULLSCREENLONG) ) 
		commandline.fullscreen=FULLSCREENON;

	if ( options_findflag(argc,argv,FLAGWINDOWED) || options_findflag(argc,argv,FLAGWINDOWEDLONG) )
	{
		if (commandline.fullscreen==FULLSCREENON)
		{	
			fprintf(stderr,"You asked for both fullscreen and windowed mode. You're mad! Mad, I tell you.\n"
			               "We'll assume \'windowed\' is what you really meant.\n");
		}
		commandline.fullscreen=FULLSCREENOFF;
	}

	// find theme name -- can't use options_findflag because this option
	// takes a parameter and that function isn't that smart.
	for(i=1; i<argc; i++)
	{
		if (strncmp(argv[i],FLAGTHEME,strlen(FLAGTHEME)) == 0)
		{
			if (strlen(argv[i])==strlen(FLAGTHEME))
			{
				fprintf(stderr,"Warning: " FLAGTHEME " option given but no theme specified. Perhaps this is because you\n"
				               " wrote \"" FLAGTHEME " themename\" or \"" FLAGTHEME "=themename\". Sorry, I'm too dumb to understand\n"
		               	               " that -- you'll have to use the form " FLAGTHEME "themename or " FLAGTHEMELONG "=themename. As it\n"
	               		               " is, I'm just going to ignore you.\n");
			}
			else if (strlen(argv[i]) > MAXTHEMENAMELENGTH+strlen(FLAGTHEME))
			{
				fprintf(stderr,"Warning: the theme name you've given is too long. The maximum is %d characters.\n"
	               		               " Not because I don't understand long filenames, by the way -- it's so the\n"
				               " options menu looks nice. I'm very fussy about my appearance.\n",MAXTHEMENAMELENGTH);
			}
			else
			{
				// fix -- we should probably search for malicious characters here.
				snprintf(commandline.theme,MAXTHEMENAMELENGTH+1,"%s",argv[i]+strlen(FLAGTHEME));
			}	
		}
		else if (strncmp(argv[i],FLAGTHEMELONG "=" ,strlen(FLAGTHEMELONG "=")) == 0)
		{
			if (strlen(argv[i])==strlen(FLAGTHEMELONG "="))
			{
				fprintf(stderr,"Warning: " FLAGTHEMELONG "= what exactly? The theme name has to go right after the equals\n"
				               " sign or else I will ignore you. Which is what I'm doing now. La la la la la la\n"
		               	               " la la la I can't hear you la la la....\n");
			}
			else if (strlen(argv[i]) > MAXTHEMENAMELENGTH+strlen(FLAGTHEMELONG "="))
			{
				fprintf(stderr,"Warning: the theme name you've given is too long. The maximum is %d characters.\n"
	               		               " Not because I don't understand long filenames, by the way -- it's so the\n"
				               " options menu looks nice. I'm very fussy about my appearance.\n",MAXTHEMENAMELENGTH);
			}
			else
			{
				// fix -- we should probably search for malicious characters here.
				snprintf(commandline.theme,MAXTHEMENAMELENGTH+1,"%s",argv[i]+strlen(FLAGTHEMELONG "="));
			}	
		}
		else if (strncmp(argv[i],FLAGTHEMELONG,strlen(FLAGTHEMELONG)) == 0)
		{
			fprintf(stderr,"Warning: the " FLAGTHEMELONG " parameter needs to be followed directly by an \'=\' and\n"
			               " then the name of the theme you want. Like, \"" FLAGTHEMELONG "=linux\". Otherwise, I\n"
	               	               " will pretend to not understand you.\n");
		}
	

	}

	return 0;
}

int options_findflag(int argc,char** argv, char* option)
{
	int i;
	for (i=1; i<argc; i++)
	{	
		//printf("[%d] = %s\n",i,argv[i]);
		if (strcmp(argv[i],option) == 0)
			return true;
	}
	return false;
}

void options_printversion()
{
	printf("\nIceBreaker v%d.%d.%d  Copyright 2000-2020 Matthew Miller, et al.\n\n",VERMAJOR,VERMINOR,VERSUB);

	printf("Written by Matthew Miller with additional code and help from Enrico Tassi\n"
	       "and others. Like to contribute to the project? Good code, graphics, sounds,\n"
       	       "and even just advice are all appreciated and often even accepted.\n\n"
	       "Visit the IceBreaker web site at <http://www.mattdm.org/icebreaker/> for more\n"
               "information or to download the latest version.\n\n"
	       "This program is free software; you can redistribute it and/or modify it\n"
               "under the terms of the GNU General Public License as published by the Free\n"
	       "Software Foundation; either version 2 of the License, or (at your option)\n"
               "any later version. IceBreaker uses the SDL library, which is distributed\n"
	       "under the GNU LGPL; see <http://www.libsdl.org/> for details.\n\n");
}
      	        
void options_printhelp()
{
	printf("Usage: icebreaker [OPTION...]\n");
	printf("%20s, %-15s %-35s\n",FLAGNOSOUND,FLAGNOSOUNDLONG,"disable game sounds");
	printf("%20s, %-15s %-35s\n",FLAGFULLSCREEN,FLAGFULLSCREENLONG,"start fullscreen if possible");
	printf("%20s, %-15s %-35s\n",FLAGWINDOWED,FLAGWINDOWEDLONG,"start windowed (resets \"Always\" option)");
	printf("%20s, %-15s %-35s\n",FLAGVERSION,FLAGVERSIONLONG,"display version and copyright info");
	printf("%20s, %-15s %-35s\n",FLAGHELP,FLAGHELPLONG,"display this help screen");
	printf("%20s, %-15s %-35s\n",FLAGLISTTHEMES,FLAGLISTTHEMESLONG,"list available themes");
	printf("%18s%s,\n","",FLAGTHEME "themename");
	printf("%18s %-18s %-35s\n","",FLAGTHEMELONG "=themename","select theme");

	printf("\nFor game play help, see the included documentation, the in-game help, or the\n"
	       "web site at <http://www.mattdm.org/icebreaker/>\n\n");
}

void options_printthemelist()
{
	char** themelist;
	int themecount;
	int t;

	printf("I've found the following themes:\n\n");

	themecount=themes_getthemenames(&themelist);
	if (themecount==0)
	{
		printf("  (none)\n");
		printf("\nThis probably means IceBreaker was installed incorrectly.\n");
	}
	else
	{
		for (t=0;t<themecount;t++)
			printf("  %s\n",themelist[t]);

		printf("\nAdditionally, you can use \'random\', which does about what you'd expect.\n");
        }
        themes_freethemenames(&themelist,themecount);
}

OPTFILEHANDLER *openoptionfile(char* filename,char* mode)
{
	OPTFILEHANDLER *rc = NULL;
	
	#ifdef WIN32
	DWORD res,err;
	rc = (OPTFILEHANDLER *)malloc(sizeof(OPTFILEHANDLER));
	rc->line = 0;
	err = RegCreateKeyExA(HKEY_CURRENT_USER,ICEBREAKER_KEY,0,NULL,REG_OPTION_NON_VOLATILE,KEY_READ|KEY_WRITE,NULL,&(rc->hK),&res);
	if(err != ERROR_SUCCESS)
	{
		fprintf(stderr,"Unable to open the regisrty\n");
		// fix - handle better
	}
	if (res == REG_CREATED_NEW_KEY)
	{
		options_closeoptionfile(rc);
		rc = NULL;
	}
	#else
	rc = fopen(filename,mode);
	#endif
	
	return rc;
}

void options_closeoptionfile(OPTFILEHANDLER *f)
{
	#ifdef WIN32
	RegCloseKey(f->hK);
	free(f);
	#else
	fclose(f);
	#endif
}

char* readoptionline(char* buffer,int len,OPTFILEHANDLER *f)
{
	char* rc = NULL;
	
	#ifdef WIN32
	DWORD err;
	char name[50],value[50];
	DWORD  name_len = 50, value_len = 50, type;
	
	err = RegEnumValueA(f->hK,f->line,name,&name_len,NULL,&type,value,&value_len);
	if(err == ERROR_NO_MORE_ITEMS )
	{
		rc = NULL;
	}	
	else if ( err != ERROR_SUCCESS )
	{
		rc = NULL;
		fprintf(stderr,"Unable to read the registry.\n");
		// fix - handle	better
	}
	else if ( type != REG_SZ )
	{
		rc = NULL;
		fprintf(stderr,"Registry key has a strange type.\n");	
	}
	else
	{
		if(name_len + value_len > len)
		{
			fprintf(stderr,"Registry key is too long for buffer\n");
			return NULL;
		}
		snprintf(buffer,len,"%s %s",name,value);
		rc = buffer;
	}
	
	f->line ++;
	#else
	rc = fgets(buffer,len,f);
	#endif
	return rc;
}

int options_writeoptionline(OPTFILEHANDLER *f,char* fmt,...)
{
	int rc = 0;
	
	#ifdef WIN32
	DWORD err;
	char name[50],value[50],line[100];
	DWORD  value_len = 0, i;
	
	va_list args;
	va_start(args, fmt);
	vsnprintf(line,100,fmt,args);
	va_end(args);
	
	if( line[0] == '#' || line[0] == '\n')
	{
		return 0; //skip comments
	}
	
	else
	
	{
		rc = strlen(line) + 1;
		//erase \n, they could be in the middle ??
		for(i = 0 ; line[i] != '\0'; i++)
			if(line[i] == '\n')
				line[i] = ' ';
		// find the separator
		for(i = 0 ; line[i] != '\0' && line[i]!=' ' ; i++);
        	
		if(line[i+1] != '\0' && line[i] == ' ')
		{
		        line[i] = '\0';
		        snprintf(name,50,"%s",line);
		        snprintf(value,50,"%s",&line[i+1]);
		        value_len = sizeof(char) * (strlen(value)+1);
			err = RegSetValueExA(f->hK,name,0,REG_SZ,value,value_len);
			if(err != ERROR_SUCCESS)
			{
				rc = 0;
				fprintf(stderr,"Unable to write the registry\n");
				// fix - handle	better
			}
		}
	
	}
	
	#else
	va_list args;
	va_start(args, fmt);
	rc = vfprintf(f,fmt,args);
	va_end(args);
	#endif
	return rc;
}
