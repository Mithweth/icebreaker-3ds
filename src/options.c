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

#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include "game.h"
#include "grid_maps.h"
#include "options.h"
#include "themes.h"
#include "lang.h"

static GameDifficultyType current_difficulty;
static GameDifficultyType selected_difficulty;
static GameMode selected_game_mode;
static GameMode current_game_mode;

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
	selected_difficulty = current_difficulty;
}

void options_difficulty_apply(void) {
    current_difficulty = selected_difficulty;
    printf("selected difficulty: %s\n", options_difficulty_serialize(current_difficulty));
}

static void options_difficulty_set(GameDifficultyType diff) {
    selected_difficulty = diff;
    current_difficulty = diff;
    printf("selected difficulty: %s\n", options_difficulty_serialize(current_difficulty));
}

GameDifficultyType options_difficulty_current(void) {
    return current_difficulty;
}

GameDifficultyType options_difficulty_selected(void) {
    return selected_difficulty;
}

void options_game_mode_next(void) {
	selected_game_mode++;
    if (selected_game_mode == GAME_MODE_COUNT) {
        selected_game_mode = 0;
    }
}

void options_game_mode_prev(void) {
    if (selected_game_mode == 0) {
        selected_game_mode = GAME_MODE_COUNT - 1;
    } else {
        selected_game_mode--;
    }
}

void options_game_mode_selected_reset(void) {
	selected_game_mode = current_game_mode;
}

void options_game_mode_apply(void) {
    current_game_mode = selected_game_mode;
    printf("selected mode: %s\n", options_game_mode_serialize(current_game_mode));
}

static void options_game_mode_set(GameMode diff) {
    selected_game_mode = diff;
    current_game_mode = diff;
    printf("selected mode: %s\n", options_game_mode_serialize(current_game_mode));
}

bool options_modern_mode(void) {
	return current_game_mode == GAME_MODE_MODERN || current_game_mode == GAME_MODE_CHALLENGE;
}

bool options_challenge_mode(void) {
	return current_game_mode == GAME_MODE_CHALLENGE;
}

GameMode options_game_mode_current(void) {
    return current_game_mode;
}

GameMode options_game_mode_selected(void) {
    return selected_game_mode;
}

const char* options_game_mode_serialize(GameMode mode) {
	switch(mode) {
	case GAME_MODE_ORIGINAL:
		return "original";
	case GAME_MODE_MODERN:
		return "modern";
	case GAME_MODE_CHALLENGE:
		return "challenge";
	default:
		return "original";
	}
}

GameMode options_game_mode_deserialize(const char *name) {
	if (strcmp(name, "original") == 0) {
		return GAME_MODE_ORIGINAL;
	} else if (strcmp(name, "modern") == 0) {
		return GAME_MODE_MODERN;
	} else if (strcmp(name, "challenge") == 0) {
		return GAME_MODE_CHALLENGE;
	} else {
		return GAME_MODE_COUNT;
	}
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

bool options_read(void) {
	FILE * optionfile;
	char linebuf[50];
	char optbuf[21];
	char valbuf[74];
	bool difficulty_found = false;
	bool theme_found = false;
	bool language_found = false;
	bool grid_found = false;
	bool mode_found = false;

	optionfile=fopen(OPTIONFILE,"r");
	while(optionfile && fgets(linebuf,50,optionfile)) {
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
				difficulty_found = true;
			}
			else if (!strcmp(optbuf,"theme"))
			{
				if (themes_exists(valbuf) && themes_settheme(valbuf, false)) {
					theme_found = true;
				}
			}
			else if (!strcmp(optbuf,"language"))
			{
				if (lang_set_current_name(valbuf)) {
					language_found = true;
				}
			}
			else if (!strcmp(optbuf,"grid"))
			{
				if (grid_maps_apply(valbuf)) {
					grid_found = true;
				}
			}
			else if (!strcmp(optbuf,"gamemode"))
			{
				GameMode mode = options_game_mode_deserialize(valbuf);
				if (mode != GAME_MODE_COUNT) {
					mode_found = true;
					options_game_mode_set(mode);
				}
			}
		}
	}
	if (!optionfile) {
		printf("Can't open %s for reading\n", OPTIONFILE);
	} else {
		fclose(optionfile);
	}
	if (!difficulty_found) {
		printf("Difficulty not found. Setting to default\n");
		options_difficulty_set(NORMAL);
	}
	if (!theme_found) {
		printf("Theme not found or not loaded. Setting to default\n");
		themes_settheme("linux", false);
	}
	if (!language_found) {
		printf("Language not found. Setting to default\n");
		lang_set_current_name("english");
	}
	if (!grid_found) {
		printf("Grid not found. Set to default\n");
		grid_maps_apply("classic");
	}
	if (!mode_found) {
		printf("Mode not found. Set to original\n");
		options_game_mode_set(GAME_MODE_ORIGINAL);
	}
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
	fprintf(optionfile,"Difficulty %s\n", options_difficulty_serialize(current_difficulty));

	fprintf(optionfile,"\n# Themes provide an easy way to select (or tweak!) the appearance\n");
	fprintf(optionfile,"# and sound of the game. Select from .ibt files in the game data\n");
	fprintf(optionfile,"# directory. Check the docs for info on creating your own themes, too.\n");	

	fprintf(optionfile,"Theme %s\n",themes_current_name());

	fprintf(optionfile,"\n# Language. Ensure a file which contains NAME=<your language>\n");
	fprintf(optionfile,"# is present in %s directory\n", LANGUAGE_DIRECTORY);	

	fprintf(optionfile,"Language %s\n",lang_current_name());

	fprintf(optionfile,"\n# Grid. If unset, will fallback to the initial behaviour.\n");
	fprintf(optionfile,"# The grid file must be .ibg file located in %s.\n", CUSTOM_GRID_DIRECTORY);
	fprintf(optionfile,"# Size must be 16x12. '#' are ground, '.' is emptiness\n");	
	const char *grid_name = grid_maps_current_name();
	if (grid_name) {
		fprintf(optionfile,"Grid %s\n", grid_name);
	} else {
		fprintf(optionfile,"# Grid classic\n");	
	}
	fprintf(optionfile,"\n# Game mode. If unset, will fallback to the 'original' mode.\n");
	fprintf(optionfile,"# Original mode is the exact same as Matthew Miller designed it.\n");
	fprintf(optionfile,"# Modern mode is a new penguin behaviour and some power-ups to collect.\n");
	fprintf(optionfile,"# Challenge mode is Modern mode with some random mechanics\n");
	fprintf(optionfile, "GameMode %s\n", options_game_mode_serialize(current_game_mode));
	fclose(optionfile);
	
	printf("options successfully written in %s\n", OPTIONFILE);
	return true;
}
