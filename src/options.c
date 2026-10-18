/*
 * IceBreaker
 * Copyright (c) 2000-2020 Matthew Miller <mattdm@mattdm.org>
 * Copyright (c) 2001-2020 Enrico Tassi <gareuselesinge@infinito.it>
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

#include <string.h>
#include <stdio.h>
#include <ctype.h>
#include "game.h"
#include "options.h"
#include "themes.h"

static GameOptionsType options;
static GameDifficultyType selected_difficulty;

void options_difficulty_next(void) {
    if (selected_difficulty == HARD) {
        selected_difficulty = EASY;
    } else {
        selected_difficulty++;
    }
}

void options_difficulty_prev(void) {
    if (selected_difficulty == EASY) {
        selected_difficulty = HARD;
    } else {
        selected_difficulty--;
    }
}

void options_difficulty_selected_reset(void) {
	selected_difficulty = options.difficulty;
}

void options_difficulty_apply(void) {
    options.difficulty = selected_difficulty;
    printf("selected difficulty: %s\n", options_difficulty_serialize(options.difficulty));
}

static void options_difficulty_set(GameDifficultyType diff) {
    selected_difficulty = diff;
    options.difficulty = diff;
    printf("selected difficulty: %s\n", options_difficulty_serialize(options.difficulty));
}

GameDifficultyType options_difficulty_current(void) {
    return options.difficulty;
}


GameDifficultyType options_difficulty_selected(void) {
    return selected_difficulty;
}

const char* options_difficulty_serialize(GameDifficultyType t) {
	switch(t) {
	case EASY:
		return "easy";
	case NORMAL:
		return "normal";
	case HARD:
		return "hard";
	default:
		return NULL;
	}
}

GameDifficultyType options_difficulty_deserialize(const char* s) {
	if (strcmp(s, "easy") == 0) {
		return EASY;
	} else if (strcmp(s, "normal") == 0) {
		return NORMAL;
	} else if (strcmp(s, "hard") == 0) {
		return HARD;
	}
	return NORMAL;
}

static bool options_setdefault(void) {
	options_difficulty_set(NORMAL);
	return themes_settheme("linux");
}

bool options_read(void) {
	FILE * optionfile;
	char linebuf[50];
	char optbuf[21];
	char valbuf[74];
	
	if (!options_setdefault()) {
		printf("Error: Cannot load default theme: linux\n");
		return false;
	}

	optionfile=fopen(OPTIONFILE,"r");
	if (!optionfile) {
		printf("Can't open %s for reading\n", OPTIONFILE);
		return false;
	}
	while(fgets(linebuf,50,optionfile)) {
		if (linebuf[0] == '\0' || linebuf[0] == '#') {
			continue;
		}
		for (int i=0;i<50;i++)
		{
			if (linebuf[i]=='\0' || linebuf[i]==' ' || linebuf[i]=='\t') break;
			linebuf[i]=tolower(linebuf[i]);
		}
		if (sscanf(linebuf,"%20s %73s",optbuf,valbuf)==2)
		{
			if (!strcmp(optbuf,"difficulty"))
			{
				options_difficulty_set(options_difficulty_deserialize(valbuf));
			}
			else if (!strcmp(optbuf,"theme"))
			{
				if (!themes_settheme(valbuf)) {
					printf("Error: Cannot load theme: %s\n", valbuf);
					fclose(optionfile);
					return false;
				}
			}
		}
	}

	fclose(optionfile);
	return true;
}

bool options_write(void) {
	FILE * optionfile;
	optionfile=fopen(OPTIONFILE,"w");
	if (!optionfile) {
		printf("Can't open %s for writing\n", OPTIONFILE);
		return false;
	}

	fprintf(optionfile,"# Icebreaker config file 1.0\n#\n");
	fprintf(optionfile,"# Separate keywords from values by whitespace. Values are case sensitive.\n#\n");
	fprintf(optionfile,"# %s will be overwritten automatically.\n#\n", OPTIONFILE);

	fprintf(optionfile,"\n# Normal is the best way to play. Easy is okay to get started,\n");
	fprintf(optionfile,"# but you won't get very high scores. Hard is for those who really\n");
	fprintf(optionfile,"# want a challenge, but scores only slightly higher than normal.\n");

	if (options.difficulty==NORMAL)
		fprintf(optionfile,"Difficulty normal\n");
	else if (options.difficulty==HARD)
		fprintf(optionfile,"Difficulty hard\n");
	else if (options.difficulty==EASY)
		fprintf(optionfile,"Difficulty easy\n");

	fprintf(optionfile,"\n# Themes provide an easy way to select (or tweak!) the appearance\n");
	fprintf(optionfile,"# and sound of the game. Select from .ibt files in the game data\n");
	fprintf(optionfile,"# directory. Check the docs for info on creating your own themes, too.\n");	

	fprintf(optionfile,"Theme %s\n",themes_current_name());

	fclose(optionfile);
	
	printf("options successfully written in %s\n", OPTIONFILE);
	return true;
}
