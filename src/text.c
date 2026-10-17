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

#include <3ds.h>
#include <citro2d.h>
#include <string.h>

#include "text.h"

static const char letterindex[] = " AAAAA BBBBB CCCCC DDDDD EEEEE FFFFF GGGGG HHHHH III JJJJJ KKKK LLLLL MMMMM NNNNN OOOOO PPPPP QQQQQ RRRRR SSSSS TTTTT UUUUU VVVVV WWWWW XXXXX YYYYY ZZZZZ aaaaa bbbbb cccc ddddd eeeee fffff ggggg hhhhh i jjjj kkkk lll mmmmm nnnnn ooooo ppppp qqqqq rrrr sssss tttt uuuuu vvvvv wwwww xxxx yyyyy zzzz 00000 11 22222 33333 44444 55555 66666 77777 88888 99999 ~~~~~    ``     !    @@@@   #####  $$$$$  %%%%%  ^^^^^  &&&&&  *****    ((   ))    _____  -----  +++++  =====   {{{     [[[   }}}   ]]]      |     :     ;;      ''   <<<<<<   ,,   >>>>>>    .    ????   /////    \"\"\" \\\\\\\\\\";

static const char *const letterdata[6] = {
	"   #   ####   ###  ####  ##### #####  #### #   # ###   ### #  # #     #   # #   #  ###  ####   ###  ####   #### ##### #   # #   # #   # #   # #   # #####       #              #         ###       #     #    # #    ##                                            #                                      ###   # ####  ####     ## #####  ###  #####  ###   ###   #       #      #     ##     # #    ###   ##  #    #     ##      #       #   #                     #              #     ###   #     ###      #                    #       ##        ##             ###        #    # # #    ",
	"  # #  #   # #   # #   # #     #     #     #   #  #      # # #  #     ## ## ##  # #   # #   # #   # #   # #       #   #   # #   # # # #  # #   # #     #   #### ####   ###  ####  ###   #     #### ####         # ##  #  ## #  # ##   ###  ####   #### # ##  #### #### #   # #   # #   # #  # #   # #### #   # ##     #     #   # # #     #         # #   # #   # # # #     #     #    # ##   #####  # #    ## #    # #   #      # # #    #     #                    #    #####    #      #      #      #      #     #      #      #      ##            ##              #      #     # #  #   ",
	" #   # ####  #     #   # ####  ####  #  ## #####  #      # ##   #     # # # # # # #   # ####  #   # ####   ###    #   #   #  # #  # # #   #     #     #   #   # #   # #    #   # ## ## ####  #   # #   # #    # ##    #  # # # ##  # #   # #   # #   # ##   ###    #   #   #  # #  # # #  ##  #   #   #  #   #  #  ###   ###   #  # ####  ####     #   ###   ####    #            #    # ##    # #    ###     #    #   #   ## #   ###     #     #           #####  #####          ##      #      ##     #      #                        ##                ##          ##      #            #  ",
	" ##### #   # #   # #   # #     #     #   # #   #  #  #   # # #  #     # # # #  ## #   # #     #   # #  #      #   #   #   #  # #  ## ##  # #    #    #    #  ## #   # #    #   # ###    #     #### #   # #    # # #   #  # # # #   # #   # #   # #   # #      ###  #   #  ##  # #  # # #  ##   ####  #   #   #  # #         # #####     # #   #   #   #   #     #                      #      #####    # #   # ##         #  #   # # #    #     #                    #    #####    #      #      #      #      #     #      #             ##      #     ##      #            #              # ",
	" #   # ####   ###  ####  ##### #      #### #   # ###  ###  #  # ##### #   # #   #  ###  #      ###  #   # ####    #    ###    #   #   # #   #   #   #####  ## # ####   ###  ####  ###   #        # #   # # #  # #  # ### #   # #   #  ###  ####   #### #    ####    ##  ## #   #    # #  #  #     # ####  ###   # ##### ####      # ####   ###   #     ###   ###                  #     ##     # #    ###   #  ##          ## #    #       #   #                     #              #     ###   #     ###      #            #               ##    #   ##        #     #     #                #",
	"                                                                                                 ##                                                                                           ###           ##                             #         #                                         ###                                                                                                                                                   #####                                                                 #                     #                                            ",
};


static int text_getletterwidth(int size, const char letter)
{
	int i;
	char * indexpointer;
	int width=0;
		
	indexpointer=index(letterindex,letter);
	if (indexpointer!=NULL)
		for (i=indexpointer-letterindex; *(letterindex+i)==letter; i++)
		{
			width+= size;
		}

	return width;
}

int text_getwidth(int size, const char * text)
{
	unsigned int l;
	int width=0;
		
	for (l=0; l<strlen(text); l++)
	{
		width += text_getletterwidth(size,*(text+l));
		width +=  CHARSPACE;
	}


	// spacing after last letter is not considered		
	width -= CHARSPACE ;

	//fprintf(stderr,"Len of '%s' with size %d is %d pixels.\n",text,size,width);
	
	return width;
}

void text_put(int x, int y, float z, int size, u32 color, const char * text)
{

	unsigned int l;
	char * indexpointer;
	unsigned char c;
	int w,startx;
	int recth = 1;
	int rectw = 1;
	
	if (size>1)
	{
		recth=(int)(size / 2.0f + 0.5f);
		rectw=(int)(size / 2.0f + 0.5f);
	}
	startx=x;
		
	for (l=0; l<strlen(text); l++)
	{
		c=*(text+l);
		indexpointer=index(letterindex,c);
		if (indexpointer!=NULL)
		{
			w=0;
			for (int i=indexpointer-letterindex; *(letterindex+i)==c; i++)
			{ // scan through index for location of letter.
				for (int j=0;j<CHARHEIGHT;j++)
				{
					if (*(letterdata[j]+i) != ' ') // hmm. maybe there's a better way to deal with spaces?
					{
						C2D_DrawRectSolid(startx+w, y+j*size, z, recth, rectw, color);
   
					}	
				}
				w+=size;
			}
			startx+=w+CHARSPACE;
		}
	}
}

void text_rput(int x, int y, float z, int size, u32 color, const char * text) {
	text_put(x - text_getwidth(size,text), y, z, size, color, text);
}

void text_cput(int x, int y, float z, int size, u32 color, const char * text) {
	text_put(x - text_getwidth(size,text) / 2, y, z, size, color, text);
}
