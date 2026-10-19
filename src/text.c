/*
 * IceBreaker
 * Copyright (c) 2000-2020 Matthew Miller <mattdm@mattdm.org>
 * Copyright (c) 2000-2020 Enrico Tassi <gareuselesinge@infinito.it>
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

#include <3ds.h>
#include <citro2d.h>
#include <string.h>
#include <math.h>

#include "text.h"

static const char letterindex[] =
	" AAAAA BBBBB CCCCC DDDDD EEEEE FFFFF GGGGG HHHHH III JJJJJ KKKK LLLLL MMMMM NNNNN OOOOO PPPPP QQQQQ RRRRR SSSSS TTTTT UUUUU VVVVV WWWWW XXXXX YYYYY ZZZZZ aaaaa \xE0\xE0\xE0\xE0\xE0 \xE4\xE4\xE4\xE4\xE4 bbbbb cccc \xE7\xE7\xE7\xE7 ddddd eeeee \xE9\xE9\xE9\xE9\xE9 \xE8\xE8\xE8\xE8\xE8 fffff ggggg hhhhh i jjjj kkkk lll mmmmm nnnnn ooooo \xF6\xF6\xF6\xF6\xF6 ppppp qqqqq rrrr sssss tttt uuuuu \xF9\xF9\xF9\xF9\xF9 \xFC\xFC\xFC\xFC\xFC vvvvv wwwww xxxx yyyyy zzzz 00000 11 22222 33333 44444 55555 66666 77777 88888 99999 ~~~~~    ``     !    @@@@   #####  $$$$$  %%%%%  ^^^^^  &&&&&  *****    ((   ))    _____  -----  +++++  =====   {{{     [[[   }}}   ]]]      |     :     ;;      ''   <<<<<<   ,,   >>>>>>    .    ????   /////    \"\"\" \\\\\\\\\\";

static const char *const letterdata[6] = {
	"   #   ####   ###  ####  ##### #####  #### #   # ###   ### #  # #     #   # #   #  ###  ####   ###  ####   #### ##### #   # #   # #   # #   # #   # #####         #     # # #                   #          #   #      ###       #     #    # #    ##                     # #                    #                 #   #   #                              ###   # ####  ####     ## #####  ###  #####  ###   ###   #       #      #     ##     # #    ###   ##  #    #     ##      #       #   #                     #              #     ###   #     ###      #                    #       ##        ##             ###        #    # # #    ",
	"  # #  #   # #   # #   # #     #     #     #   #  #      # # #  #     ## ## ##  # #   # #   # #   # #   # #       #   #   # #   # # # #  # #   # #     #   ####  ####  #### ####   ###  ###  ####  ###   ###   ###   #     #### ####         # ##  #  ## #  # ##   ###   ###  ####   #### # ##  #### #### #   # #   #       #   # #   # #  # #   # #### #   # ##     #     #   # # #     #         # #   # #   # # # #     #     #    # ##   #####  # #    ## #    # #   #      # # #    #     #                    #    #####    #      #      #      #      #     #      #      #      ##            ##              #      #     # #  #   ",
	" #   # ####  #     #   # ####  ####  #  ## #####  #      # ##   #     # # # # # # #   # ####  #   # ####   ###    #   #   #  # #  # # #   #     #     #   #   # #   # #   # #   # #    #    #   # ## ## ## ## ## ## ####  #   # #   # #    # ##    #  # # # ##  # #   # #   # #   # #   # ##   ###    #   #   # #   # #   #  # #  # # #  ##  #   #   #  #   #  #  ###   ###   #  # ####  ####     #   ###   ####    #            #    # ##    # #    ###     #    #   #   ## #   ###     #     #           #####  #####          ##      #      ##     #      #                        ##                ##          ##      #            #  ",
	" ##### #   # #   # #   # #     #     #   # #   #  #  #   # # #  #     # # # #  ## #   # #     #   # #  #      #   #   #   #  # #  ## ##  # #    #    #    #  ## #  ## #  ## #   # #    #    #   # ###   ###   ###    #     #### #   # #    # # #   #  # # # #   # #   # #   # #   # #   # #      ###  #   #  ## #  ## #  ##  # #  # # #  ##   ####  #   #   #  # #         # #####     # #   #   #   #   #     #                      #      #####    # #   # ##         #  #   # # #    #     #                    #    #####    #      #      #      #      #     #      #             ##      #     ##      #            #              # ",
	" #   # ####   ###  ####  ##### #      #### #   # ###  ###  #  # ##### #   # #   #  ###  #      ###  #   # ####    #    ###    #   #   # #   #   #   #####  ## #  ## #  ## # ####   ###  ###  ####  ###   ###   ###   #        # #   # # #  # #  # ### #   # #   #  ###   ###  ####   #### #    ####    ##  ## #  ## #  ## #   #    # #  #  #     # ####  ###   # ##### ####      # ####   ###   #     ###   ###                  #     ##     # #    ###   #  ##          ## #    #       #   #                     #              #     ###   #     ###      #            #               ##    #   ##        #     #     #                #",
	"                                                                                                 ##                                                                                      #                                 ###           ##                                   #         #                                                     ###                                                                                                                                                   #####                                                                 #                     #                                            ",
};

static unsigned int text_next_char(const unsigned char **text) {
	unsigned int c = **text;
	(*text)++;

	if (c == 0xC3 && (**text & 0xC0) == 0x80) {
		unsigned int first = (c & 0x1F) << 6;
		unsigned int second = **text & 0x3F;
		(*text)++;
		c = first | second;
	}

	return c;
}

// Rewritten it all: UTF-8 is a bitch. Thank anyway, stack overflow ;-)
static int text_getletterwidth(int size, unsigned int letter) {
	const char *indexpointer = strchr(letterindex, (unsigned char)letter);
	if (!indexpointer) return 0;

	int width = 0;
	for (int i = indexpointer - letterindex; (unsigned char)letterindex[i] == letter; i++) width += size;

	return width;
}

int text_getwidth(int size, const char *text) {
	const unsigned char *p = (const unsigned char *)text;
	int width = 0;

	while (*p) {
		unsigned int c = text_next_char(&p);
		if (strchr(letterindex, (unsigned char)c)) width += text_getletterwidth(size, c) + CHARSPACE;
	}

	if (width) width -= CHARSPACE;
	return width;
}

// Updated the function to use float instead of int because the 3DS slider returns
// a float values between 0.0f and 1.0f being rounded by integers when the slider
// is less than 80%
void text_put(float x, float y, float z, float size, u32 color, const char * text)
{
	float w, startx;
	float recth = 1.0f;
	float rectw = 1.0f;
	
	if (size>1)
	{
		recth = floorf(size / 2.0f + 0.5f);
		rectw = floorf(size / 2.0f + 0.5f);
	}
	startx = floorf(x + 0.5f);
	y = floorf(y + 0.5f);
	const unsigned char *p = (const unsigned char *)text;
	while (*p) {
		unsigned int c = text_next_char(&p);
		const char *indexpointer = strchr(letterindex, (unsigned char)c);
		if (indexpointer!=NULL)
		{
			w=0;
			for (int i=indexpointer-letterindex; (unsigned char)letterindex[i]==c; i++)
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

void text_rput(float x, float y, float z, float size, u32 color, const char * text) {
	text_put(x - text_getwidth(size,text), y, z, size, color, text);
}

void text_cput(float x, float y, float z, float size, u32 color, const char * text) {
	text_put(x - text_getwidth(size,text) / 2, y, z, size, color, text);
}
