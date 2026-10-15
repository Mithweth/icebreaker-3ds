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

ThemeColorsType color;
C2D_Image spriteimage;

static void themes_setdefaultcolors(void)
{
	color.background      = C2D_Color32(  0, 64,128,255);
	color.normaltext      = C2D_Color32(255,255,255,255);
	color.gridline        = C2D_Color32(192,192,192,255);
	color.gridhighlight   = C2D_Color32(192,192,192,255);
	color.line1           = C2D_Color32(  0,  0,  0,255);
	color.line2           = C2D_Color32(192,  0, 64,255);
	color.menuhighlight   = C2D_Color32(240,240,240,255);
	color.gameovertext    = C2D_Color32(192,  0, 64,255);
	color.scorescrolltext = C2D_Color32(  0,  0,  0,255);
	color.bonusscrolltext = C2D_Color32(192,  0, 64,255);
	color.textentrybox    = C2D_Color32(  0,  0,  0,255);
	color.textentrytext   = C2D_Color32(255,255,255,255);
	color.copyrighttext   = C2D_Color32(192,192,192,255);

	color.boardfillminr = 224;
 	color.boardfillming = 224;
	color.boardfillminb = 224;
	color.boardfillmaxr = 255;
	color.boardfillmaxg = 255;
	color.boardfillmaxb = 255;
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
	int r,g,b;
	ThemeColorsType foundcolor;
	
	// if (!strcmp(themename,"random"))
	// {
	// 	return themes_setrandomtheme(); // whooo recursion!
	// }
	
	themes_setdefaultcolors();
	if (spriteimage.tex)
	{
		spriteimage=(C2D_Image){0};
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
				//printf("N: %s = %d,%d,%d\n",optbuf,r,g,b);
				
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
					color.background=C2D_Color32(r,g,b,255);
					foundcolor.background++;
				}
				else if (!strcmp(optbuf,"normaltext"))
				{
					color.normaltext=C2D_Color32(r,g,b,255);
					foundcolor.normaltext++;
				}
				else if (!strcmp(optbuf,"boardfillmin"))
				{
					color.boardfillminr=r;
					color.boardfillming=g;
					color.boardfillminb=b;
					foundcolor.boardfillminr++;
					foundcolor.boardfillming++;
					foundcolor.boardfillminb++;
					
				}
				else if (!strcmp(optbuf,"boardfillmax"))
				{
					color.boardfillmaxr=r;
					color.boardfillmaxg=g;
					color.boardfillmaxb=b;
					foundcolor.boardfillmaxr++;
					foundcolor.boardfillmaxg++;
					foundcolor.boardfillmaxb++;
				}
				else if (!strcmp(optbuf,"gridline"))
				{
					color.gridline=C2D_Color32(r,g,b,255);
					foundcolor.gridline++;
				}
				else if (!strcmp(optbuf,"gridhighlight"))
				{
					color.gridhighlight=C2D_Color32(r,g,b,255);
					foundcolor.gridhighlight++;
				}
				else if (!strcmp(optbuf,"line1"))
				{
					color.line1=C2D_Color32(r,g,b,255);
					foundcolor.line1++;
				}
				else if (!strcmp(optbuf,"line2"))
				{
					color.line2=C2D_Color32(r,g,b,255);
					foundcolor.line2++;
				}
				else if (!strcmp(optbuf,"menuhighlight"))
				{
					color.menuhighlight=C2D_Color32(r,g,b,255);
					foundcolor.menuhighlight++;

				}
				else if (!strcmp(optbuf,"gameovertext"))
				{
					color.gameovertext=C2D_Color32(r,g,b,255);
					foundcolor.gameovertext++;
				}
				else if (!strcmp(optbuf,"scorescrolltext"))
				{
					color.scorescrolltext=C2D_Color32(r,g,b,255);
					foundcolor.scorescrolltext++;
				}
				else if (!strcmp(optbuf,"bonusscrolltext"))
				{
					color.bonusscrolltext=C2D_Color32(r,g,b,255);
					foundcolor.bonusscrolltext++;
				}
				else if (!strcmp(optbuf,"textentrybox"))
				{
					color.textentrybox=C2D_Color32(r,g,b,255);
					foundcolor.textentrybox++;

				}
				else if (!strcmp(optbuf,"textentrytext"))
				{
					color.textentrytext=C2D_Color32(r,g,b,255);
					foundcolor.textentrytext++;
				}
				else if (!strcmp(optbuf,"copyrighttext"))
				{
					color.copyrighttext=C2D_Color32(r,g,b,255);
					foundcolor.copyrighttext++;

				}
			}
			else if (sscanf(linebuf,"%20s %255s",optbuf,valbuf)==2)
			{
				//printf("S: %s = %s\n",optbuf,valbuf);
				if (!strcmp(optbuf,"spritebitmap"))
				{
					spriteimage=gfxmap_get_image(valbuf);
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
	
	if (color.boardfillmaxr<color.boardfillminr)
	{
		color.boardfillmaxr=color.boardfillminr;
		fprintf(stderr,"Warning: max red value less than min value in theme %s\n",themename);
	}
	if (color.boardfillmaxg<color.boardfillming)
	{
		color.boardfillmaxg=color.boardfillming;
		fprintf(stderr,"Warning: max green value less than min value in theme %s\n",themename);
	}
	if (color.boardfillmaxb<color.boardfillminb)
	{
		color.boardfillmaxb=color.boardfillminb;
		fprintf(stderr,"Warning: max blue value less than min value in theme %s\n",themename);
	}
	

	if (!spriteimage.tex)
	{
		rc=-255;
	}
		
	return rc;
}