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
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "game.h"
#include "hiscore.h"
#include "text.h"
#include "themes.h"
#include "lang.h"
#include "grid_maps.h"
#include "options.h"

// +1 to avoid memory issue from random mode
static HighScore hiscore[GAME_MODE_COUNT][MAX_GRIDS + 1][HISCORENUM];
static char username[50];

static int hiscore_cmpscore(const void *a, const void *b) {
	long x = ((const HighScore *)a)->score;
	long y = ((const HighScore *)b)->score;
	return (x < y) - (x > y);
}

void hiscore_read(void) {
	FILE *hiscorefile;
	char linebuf[128];
	char gridname[32];
	char game_mode[9];
	char name[50];
	long score;
	// make sure all entries are zeroed out to start.
	for (size_t m = 0; m < GAME_MODE_COUNT; m++) {
		for (size_t g = 0; g < grid_maps_count(); g++) {
			for (int i = 0; i < HISCORENUM; i++) {
				snprintf(hiscore[m][g][i].name, sizeof(hiscore[m][g][i].name), "Nobody");
				hiscore[m][g][i].score = 100; //100 is better than 0. :)
			}
		}
	}

	hiscorefile=fopen(HISCOREFILE,"r");
	if (!hiscorefile) {
		printf("Can't open %s for reading\n", HISCOREFILE);
		return;
	}

	while (fgets(linebuf, sizeof(linebuf), hiscorefile)) {
		GameMode mode = GAME_MODE_COUNT;
		if (sscanf(linebuf, "%8s %31s %ld %49[^\r\n]", game_mode, gridname, &score, name) == 4) {
			mode = options_game_mode_deserialize(game_mode);
		}
		if (mode != GAME_MODE_COUNT) {
			hiscore_add(mode, gridname, name, score);
		} else if (sscanf(linebuf, "%31s %ld %49[^\r\n]", gridname, &score, name) == 3) {
			hiscore_add(GAME_MODE_ORIGINAL, gridname, name, score); // old file format
		}
	}
	fclose(hiscorefile);
}

HighScore* hiscore_get(GameMode mode, const char *grid, int id) {
	int g = grid_maps_get_index(grid);
	if (g < 0 || id < 0 || id >= HISCORENUM) return NULL;
	return &hiscore[mode][g][id];
}

bool hiscore_check_high(GameMode mode, const char *grid, long score) {
	// check to see if score is better than the lowest high score
	return score > hiscore_get(mode, grid, HISCORENUM - 1)->score;
}

void hiscore_add(GameMode mode, const char *grid, const char *name, long score) {
	int g = grid_maps_get_index(grid);
	if (g < 0 || score <= hiscore[mode][g][HISCORENUM - 1].score) return;

	HighScore *worst = &hiscore[mode][g][HISCORENUM - 1];
	snprintf(worst->name, sizeof(worst->name), "%s", name);
	worst->score = score;

	qsort(hiscore[mode][g], HISCORENUM, sizeof(HighScore), hiscore_cmpscore);
}

void hiscore_write(void) {
	FILE *hiscorefile;
	char filename[274];

	snprintf(filename, 274, "%s", HISCOREFILE);
	
	hiscorefile = fopen(filename, "w");
	if (!hiscorefile) {
		printf("Can't open %s for writing\n", filename);
		return;
	}
	for(size_t m = 0; m < GAME_MODE_COUNT; m++) {
		for (size_t g = 0; g < grid_maps_count(); g++) {
			if (grid_maps_random(grid_maps_get_name(g))) continue;
			for (int i = 0; i < HISCORENUM; i++) {
				fprintf(hiscorefile, "%s %s %ld %s\n", options_game_mode_serialize(m), grid_maps_get_name(g), hiscore[m][g][i].score, hiscore[m][g][i].name);
			}
		}
	}
	printf("Written highscores in %s successfully\n", filename);
	fclose(hiscorefile);
}

static bool system_get_username(char *name, size_t size) {
	if (size == 0) return false;
	name[0] = '\0';

	u8 block[0x1C];
	u16 username[12] = {0};

	if (R_FAILED(cfguInit())) return false;
	Result rc = CFGU_GetConfigInfoBlk2(sizeof(block), 0x000A0000, block);
	cfguExit();
	if (R_FAILED(rc)) return false;

	memcpy(username, block, 0x16);

	ssize_t len = utf16_to_utf8((u8 *)name, username, size - 1);
	if (len < 0 || (size_t)len >= size) {
		name[0] = '\0';
		return false;
	}

	name[len] = '\0';
	return len > 0;
}

static bool hiscore_prompt_name(void) {
	SwkbdState keyboard;
	char new_username[sizeof(username)];
	swkbdInit(&keyboard, SWKBD_TYPE_NORMAL, 2, 12);
	swkbdSetHintText(&keyboard, lang_get("SWKB_PROMPT"));
	swkbdSetInitialText(&keyboard, username);
	swkbdSetValidation(&keyboard, SWKBD_NOTEMPTY_NOTBLANK, 0, 0);
	swkbdSetButton(&keyboard, SWKBD_BUTTON_LEFT, lang_get("SWKB_CANCEL"), false);
	swkbdSetButton(&keyboard, SWKBD_BUTTON_RIGHT, lang_get("SWKB_OK"), true);

	if (swkbdInputText(&keyboard, new_username, sizeof(new_username)) != SWKBD_BUTTON_RIGHT) return false;
	strcpy(username, new_username);
	return true;
}

void hiscore_dialog_init(void) {
	if (!system_get_username(username, sizeof(username)) || username[0] == '\0') {
		snprintf(username, sizeof(username), "Nobody");
	}
}

void hiscore_dialog_draw(void) {
	C2D_DrawRectSolid(0, 0, 0.4f, WIDTH, HEIGHT, C2D_Color32(0, 0, 0, 192));
	C2D_DrawRectSolid(50.0f, 50.0f, 0.41f, 220.0f, 140.0f, theme.color.menuhighlight);
	C2D_DrawRectSolid(52.0f, 52.0f, 0.42f, 216.0f, 136.0f, theme.color.background);
	text_put(60, 60, 0.43f, 2, theme.color.normaltext, lang_get("HISCORE_NEW_HIGH"));
	text_put(60, 80, 0.43f, 2, theme.color.normaltext, lang_get("HISCORE_PROMPT"));
	C2D_DrawRectSolid(60.0f, 102.0f, 0.43f, 200.0f, 21.0f, theme.color.textentrybox);
	text_put(70, 104, 0.44f, 3, theme.color.textentrytext, username);

	int box_w = 20.0f + text_getwidth(3, lang_get("CHOICE_OK"));
	C2D_DrawRectSolid(200.0f, 160.0f, 0.43f, box_w, 18.0f, theme.color.menuhighlight);
	text_cput(200 + box_w / 2, 162, 0.44f, 3, theme.color.background, lang_get("CHOICE_OK"));
}

bool hiscore_dialog_update(u32 keys, touchPosition touch, const GameStats *stats) {
	if (keys & KEY_A) {
		hiscore_add(options_game_mode(), grid_maps_current_name(), username, stats->score);
		hiscore_write();
		return true;
	}

	if (!(keys & KEY_TOUCH)) {
		return false;
	}
	if (touch.px >= 60 && touch.px <= 260 && touch.py >= 96 && touch.py <= 124) {
		hiscore_prompt_name();
	}
	if (touch.px >= 210 && touch.px <= 260 && touch.py >= 160 && touch.py <= 178) {
		hiscore_add(options_game_mode(), grid_maps_current_name(), username, stats->score);
		hiscore_write();
		return true;
	}
	return false;
}
