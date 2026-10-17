/*
* IceBreaker
* Copyright (c) 2000-2020 Matthew Miller <mattdm@mattdm.org> and 
*  Enrico Tassi <gareuselesinge@infinito.it>
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

#include <stdio.h>
#include <stdlib.h>
#include "game.h"
#include "hiscore.h"

static HighScore hiscore[HISCORENUM];

static int hiscore_cmpscore(const void *a, const void *b) {
    long x = ((const HighScore *)a)->score;
    long y = ((const HighScore *)b)->score;
    return (x < y) - (x > y);
}

void hiscore_read(void) {
	FILE *hiscorefile;
	char linebuf[80];
	char filename[274];

	// make sure all entries are zeroed out to start.
	for (int i=0;i<HISCORENUM;i++)
	{
		snprintf(hiscore[i].name,7,"Nobody");
		hiscore[i].score=100; //100 is better than 0. :)
	}

	snprintf(filename,274,"%s",HISCOREFILE);

	hiscorefile=fopen(filename,"r");
	if (hiscorefile) {
		for (int i=0;i<HISCORENUM;i++)
		{
			if (fgets(linebuf,80,hiscorefile))
			{
				sscanf(linebuf,"%49s %30ld",hiscore[i].name,&hiscore[i].score);
			}
		}
		fclose(hiscorefile);
		
		qsort(hiscore, HISCORENUM, sizeof(hiscore[0]), hiscore_cmpscore);
	} else {
		printf("Can't open %s for reading\n", filename);
	}
}

bool hiscore_check_high(long thisgamescore) {
	// check to see if score is better than the lowest high score
	return (thisgamescore>hiscore[0].score);
}

HighScore* hiscore_get(int id) {
	if (id < 0 || id >= HISCORENUM) {
		return NULL;
	}
	return &hiscore[id];
}

void hiscore_add(const char* name, long score) {
    HighScore tmp[HISCORENUM + 1];

    memcpy(tmp, hiscore, sizeof(hiscore));
    snprintf(tmp[HISCORENUM].name,50,"%s",name);
    tmp[HISCORENUM].score = score;

    qsort(tmp, HISCORENUM + 1, sizeof(tmp[0]), hiscore_cmpscore);

    memcpy(hiscore, tmp, sizeof(hiscore));
}

void hiscore_write(void) {
	FILE *hiscorefile;
	char filename[274];

	snprintf(filename,274,"%s",HISCOREFILE);
	
	hiscorefile=fopen(filename,"w");
	if (!hiscorefile) {
		printf("Can't open %s for writing\n", filename);
		return;
	}
	// FIX -- make this go so people can have spaces in their names
	//for (i=0;hiscorename[i]!='\0';i++)
	//      if (hiscorename[i]==' ') hiscorename[i]='_';

	for (int i=0;i<HISCORENUM;i++) {
		fprintf(hiscorefile,"%s %ld\n",hiscore[i].name,hiscore[i].score);
	}

	fclose(hiscorefile);
}
