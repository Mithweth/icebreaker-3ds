/*
* IceBreaker
* Copyright (c) 2002-2020 Matthew Miller <mattdm@mattdm.org>
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

#include <citro2d.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <dirent.h>
#include <string.h>
#include <ctype.h>
#include "themes.h"
#include "gfxmap.h"

Theme theme;

static void themes_setdefaultcolors(void)
{
	theme.color.background      = C2D_Color32(  0, 64,128,255);
	theme.color.normaltext      = C2D_Color32(255,255,255,255);
	theme.color.gridline        = C2D_Color32(192,192,192,255);
	theme.color.gridhighlight   = C2D_Color32(192,192,192,255);
	theme.color.line1           = C2D_Color32(  0,  0,  0,255);
	theme.color.line2           = C2D_Color32(192,  0, 64,255);
	theme.color.menuhighlight   = C2D_Color32(240,240,240,255);
	theme.color.gameovertext    = C2D_Color32(192,  0, 64,255);
	theme.color.scorescrolltext = C2D_Color32(  0,  0,  0,255);
	theme.color.bonusscrolltext = C2D_Color32(192,  0, 64,255);
	theme.color.textentrybox    = C2D_Color32(  0,  0,  0,255);
	theme.color.textentrytext   = C2D_Color32(255,255,255,255);
	theme.color.copyrighttext   = C2D_Color32(192,192,192,255);
	theme.color.progressbaroutline = C2D_Color32(180, 225, 245, 255);
	theme.color.progressbarinner = C2D_Color32(100, 200, 240, 255);
	theme.color.boardfillminr = 224;
 	theme.color.boardfillming = 224;
	theme.color.boardfillminb = 224;
	theme.color.boardfillmaxr = 255;
	theme.color.boardfillmaxg = 255;
	theme.color.boardfillmaxb = 255;
}

int themes_settheme(char* themename)
{
	int rc=0;
	int i;
	FILE* themefile;
	char linebuf[300];
	char themefilename[256]; // fix -- use defined OS constant (here and throughout this function)
	char optbuf[21];
	char valbuf[256];
	snprintf(theme.name, 64, "%s", themename);
	int r,g,b;
	ThemeColorsType foundcolor;
	
	// if (!strcmp(themename,"random"))
	// {
	// 	return themes_setrandomtheme(); // whooo recursion!
	// }
	
	themes_setdefaultcolors();
	if (theme.spriteimage.tex)
	{
		theme.spriteimage=(C2D_Image){0};
	}
	
	foundcolor.background      = 0;
	foundcolor.normaltext      = 0;
	foundcolor.gridline        = 0;
	foundcolor.gridhighlight   = 0;
	foundcolor.line1           = 0;
	foundcolor.line2           = 0;
	foundcolor.menuhighlight   = 0;
	foundcolor.gameovertext    = 0;
	foundcolor.scorescrolltext = 0;
	foundcolor.bonusscrolltext = 0;
	foundcolor.textentrybox    = 0;
	foundcolor.textentrytext   = 0;
	foundcolor.copyrighttext   = 0;
	foundcolor.progressbaroutline = 0;
	foundcolor.progressbarinner = 0;
	foundcolor.boardfillminr = 0;
 	foundcolor.boardfillming = 0;
	foundcolor.boardfillminb = 0;
	foundcolor.boardfillmaxr = 0;
	foundcolor.boardfillmaxg = 0;
	foundcolor.boardfillmaxb = 0;

	snprintf(themefilename, 256,"romfs:/themes/%s.ibt",themename);
	
	themefile=fopen(themefilename,"r");
	if (themefile==NULL)
	{
		// FIX -- ??? question: should this actually change the
		// current setting, or is leaving it at the currently-unavailable
		// value the right thing to do?
		fprintf(stderr, "Warning: Can't open theme file %s.\n"
		                "Using default look and sound.\n",themefilename);
		rc=-18;		               
	}
	else
	{
		while(fgets(linebuf,300,themefile))
		{
			for (i=0;i<300;i++)
			{	
				if (linebuf[i]=='\0' || linebuf[i]==' ' || linebuf[i]=='\t') break;
				linebuf[i]=tolower(linebuf[i]);
			}
			
			if (sscanf(linebuf,"%20s %5d %5d %5d",optbuf,&r,&g,&b)==4)
			{
				if (r<0 || r>255)
				{
					fprintf(stderr,"Warning: invalid red value for %s in theme %s\n",optbuf,themename);
					r=0;
				}
				if (g<0 || g>255)
				{
					fprintf(stderr,"Warning: invalid green value for %s in theme %s\n",optbuf,themename);
					g=0;
				}
				if (b<0 || b>255)
				{
					fprintf(stderr,"Warning: invalid blue value for %s in theme %s\n",optbuf,themename);
					b=0;
				}
				
				if (!strcmp(optbuf,"background"))
				{
					theme.color.background=C2D_Color32(r,g,b,255);
					foundcolor.background++;
				}
				else if (!strcmp(optbuf,"normaltext"))
				{
					theme.color.normaltext=C2D_Color32(r,g,b,255);
					foundcolor.normaltext++;
				}
				else if (!strcmp(optbuf,"boardfillmin"))
				{
					theme.color.boardfillminr=r;
					theme.color.boardfillming=g;
					theme.color.boardfillminb=b;
					foundcolor.boardfillminr++;
					foundcolor.boardfillming++;
					foundcolor.boardfillminb++;
					
				}
				else if (!strcmp(optbuf,"boardfillmax"))
				{
					theme.color.boardfillmaxr=r;
					theme.color.boardfillmaxg=g;
					theme.color.boardfillmaxb=b;
					foundcolor.boardfillmaxr++;
					foundcolor.boardfillmaxg++;
					foundcolor.boardfillmaxb++;
				}
				else if (!strcmp(optbuf,"gridline"))
				{
					theme.color.gridline=C2D_Color32(r,g,b,255);
					foundcolor.gridline++;
				}
				else if (!strcmp(optbuf,"gridhighlight"))
				{
					theme.color.gridhighlight=C2D_Color32(r,g,b,255);
					foundcolor.gridhighlight++;
				}
				else if (!strcmp(optbuf,"line1"))
				{
					theme.color.line1=C2D_Color32(r,g,b,255);
					foundcolor.line1++;
				}
				else if (!strcmp(optbuf,"line2"))
				{
					theme.color.line2=C2D_Color32(r,g,b,255);
					foundcolor.line2++;
				}
				else if (!strcmp(optbuf,"menuhighlight"))
				{
					theme.color.menuhighlight=C2D_Color32(r,g,b,255);
					foundcolor.menuhighlight++;
				}
				else if (!strcmp(optbuf,"gameovertext"))
				{
					theme.color.gameovertext=C2D_Color32(r,g,b,255);
					foundcolor.gameovertext++;
				}
				else if (!strcmp(optbuf,"scorescrolltext"))
				{
					theme.color.scorescrolltext=C2D_Color32(r,g,b,255);
					foundcolor.scorescrolltext++;
				}
				else if (!strcmp(optbuf,"bonusscrolltext"))
				{
					theme.color.bonusscrolltext=C2D_Color32(r,g,b,255);
					foundcolor.bonusscrolltext++;
				}
				else if (!strcmp(optbuf,"textentrybox"))
				{
					theme.color.textentrybox=C2D_Color32(r,g,b,255);
					foundcolor.textentrybox++;

				}
				else if (!strcmp(optbuf,"textentrytext"))
				{
					theme.color.textentrytext=C2D_Color32(r,g,b,255);
					foundcolor.textentrytext++;
				}
				else if (!strcmp(optbuf,"copyrighttext"))
				{
					theme.color.copyrighttext=C2D_Color32(r,g,b,255);
					foundcolor.copyrighttext++;
				}
				else if (!strcmp(optbuf,"progressbaroutline"))
				{
					theme.color.progressbaroutline=C2D_Color32(r,g,b,255);
					foundcolor.progressbaroutline++;
				}
				else if (!strcmp(optbuf,"progressbarinner"))
				{
					theme.color.progressbarinner=C2D_Color32(r,g,b,255);
					foundcolor.progressbarinner++;
				}
			}
			else if (sscanf(linebuf,"%20s %255s",optbuf,valbuf)==2)
			{
				//printf("S: %s = %s\n",optbuf,valbuf);
				if (!strcmp(optbuf,"spritebitmap"))
				{
					theme.spriteimage=gfxmap_get_image(valbuf);
				}
				// else if (!strcmp(optbuf,"soundouch"))
				// {
				// 	if (strcasecmp( ".wav", valbuf+(strlen(valbuf)-strlen(".wav")) ))
				// 	{
				// 		fprintf(stderr,"Warning: Ouch sound file in theme %s does not end with '.wav'.\n"
				// 		               "We'll try to load it anyway, but don't be surpised if there's a problem.\n"
				// 		               "(Filename is %s)\n",themename,valbuf);
				// 	}
				// 	snprintf(loadfilebuf, 256,"%s/%s",DATAPREFIX,valbuf);
				// 	sound_loads(themename,loadfilebuf,NULL);
				// }
				// else if (!strcmp(optbuf,"soundcrash"))
				// {
				// 	if (strcasecmp( ".wav", valbuf+(strlen(valbuf)-strlen(".wav")) ))
				// 	{
				// 		fprintf(stderr,"Warning: Crash sound file in theme %s does not end with '.wav'.\n"
				// 		               "We'll try to load it anyway, but don't be surpised if there's a problem.\n"
				// 		               "(Filename is %s)\n",themename,valbuf);
				// 	}
				// 	snprintf(loadfilebuf, 256,"%s/%s",DATAPREFIX,valbuf);
				// 	sound_loads(themename,NULL,loadfilebuf);
				// }
			}
			
		}	
		fclose(themefile);
	}


	if (foundcolor.background == 0)
		{ rc--; fprintf(stderr,"Warning: Background not correctly defined in theme %s. Using default.\n",themename); }
	if (foundcolor.normaltext == 0)
		{ rc--; fprintf(stderr,"Warning: NormalText not correctly defined in theme %s. Using default.\n",themename); }
	if (foundcolor.boardfillminr == 0)
		{ rc--; fprintf(stderr,"Warning: BoardFillMin not correctly defined in theme %s. Using default.\n",themename); }
	if (foundcolor.boardfillmaxr == 0)
		{ rc--; fprintf(stderr,"Warning: BoardFillMax not correctly defined in theme %s. Using default.\n",themename); }
	if (foundcolor.gridline == 0)
		{ rc--; fprintf(stderr,"Warning: Gridline not correctly defined in theme %s. Using default.\n",themename); }
	if (foundcolor.gridhighlight == 0)
		{ rc--; fprintf(stderr,"Warning: GridHighlight not correctly defined in theme %s. Using default.\n",themename); }
	if (foundcolor.line1 == 0)
		{ rc--; fprintf(stderr,"Warning: Line1 not correctly defined in theme %s. Using default.\n",themename); }
	if (foundcolor.line2 == 0)
		{ rc--; fprintf(stderr,"Warning: Line2 not correctly defined in theme %s. Using default.\n",themename); }
	if (foundcolor.menuhighlight == 0)
		{ rc--; fprintf(stderr,"Warning: MenuHighlight not correctly defined in theme %s. Using default.\n",themename); }
	if (foundcolor.gameovertext == 0)
		{ rc--; fprintf(stderr,"Warning: GameoverText not correctly defined in theme %s. Using default.\n",themename); }
	if (foundcolor.scorescrolltext == 0)
		{ rc--; fprintf(stderr,"Warning: ScorescrollText not correctly defined in theme %s. Using default.\n",themename); }
	if (foundcolor.bonusscrolltext == 0)
		{ rc--; fprintf(stderr,"Warning: BonusscrollText not correctly defined in theme %s. Using default.\n",themename); }
	if (foundcolor.textentrybox == 0)
		{ rc--; fprintf(stderr,"Warning: TextentryBox not correctly defined in theme %s. Using default.\n",themename); }
	if (foundcolor.textentrytext == 0)
		{ rc--; fprintf(stderr,"Warning: TextentryText not correctly defined in theme %s. Using default.\n",themename); }
	if (foundcolor.copyrighttext == 0)
		{ rc--; fprintf(stderr,"Warning: CopyrightText not correctly defined in theme %s. Using default.\n",themename); }
	if (foundcolor.progressbaroutline == 0)
		{ rc--; fprintf(stderr,"Warning: ProgressbarOutline not correctly defined in theme %s. Using default.\n",themename); }
	if (foundcolor.progressbarinner == 0)
		{ rc--; fprintf(stderr,"Warning: ProgressbarInner not correctly defined in theme %s. Using default.\n",themename); }
	
	if (theme.color.boardfillmaxr<theme.color.boardfillminr)
	{
		theme.color.boardfillmaxr=theme.color.boardfillminr;
		fprintf(stderr,"Warning: max red value less than min value in theme %s\n",themename);
	}
	if (theme.color.boardfillmaxg<theme.color.boardfillming)
	{
		theme.color.boardfillmaxg=theme.color.boardfillming;
		fprintf(stderr,"Warning: max green value less than min value in theme %s\n",themename);
	}
	if (theme.color.boardfillmaxb<theme.color.boardfillminb)
	{
		theme.color.boardfillmaxb=theme.color.boardfillminb;
		fprintf(stderr,"Warning: max blue value less than min value in theme %s\n",themename);
	}
	
	if (!theme.spriteimage.tex)
	{
		rc=-255;
	}
	printf("Theme loading return code: %d\n", rc);
	return rc;
}