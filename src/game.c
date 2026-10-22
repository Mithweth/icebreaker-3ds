/*
 * IceBreaker - Nintendo 3DS port
 * Copyright (c) 2026 Jean-Baptiste Langlois
 *
 * This file is part of IceBreaker.
 *
 * <https://github.com/Mithweth/icebreaker-3ds>
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
#include <stdbool.h>
#include <string.h>
#include <sys/stat.h>
#include <errno.h>
#include "gfxmap.h"
#include "intro.h"
#include "grid_maps.h"
#include "themes.h"
#include "level.h"
#include "menu.h"
#include "intermission.h"
#include "sound.h"
#include "hiscore.h"
#include "options.h"
#include "snowflake.h"
#include "editor.h"

typedef enum {
	GAME_INTRO,
	GAME_LEVEL,
	GAME_HIGH_SCORE,
	GAME_EDITOR,
	GAME_INTERMISSION,
	GAME_OVER
} GameMode;

static GameMode game_mode;
static GameMode previous_game_mode;

static bool pause;
static GameStats stats;

static bool ensure_data_dir(void) {
	if (mkdir(CUSTOM_GRID_DIRECTORY, 0777) != 0 && errno != EEXIST) {
		printf("Can't create directory %s\n", CUSTOM_GRID_DIRECTORY);
		return false;
	}
	char path_hiscore[] = HISCOREFILE;
	char *slash = strrchr(path_hiscore, '/');
	if (slash) {
		*slash = '\0';
		if (mkdir(path_hiscore, 0777) != 0 && errno != EEXIST) {
			printf("Can't create directory %s\n", path_hiscore);
			return false;
		}
	}
	char path_options[] = OPTIONFILE;
	slash = strrchr(path_options, '/');
	if (slash) {
		*slash = '\0';
		if (mkdir(path_options, 0777) != 0 && errno != EEXIST) {
			printf("Can't create directory %s\n", path_options);
			return false;
		}
	}
	return true;
}

bool game_init(void) {
	ensure_data_dir();
	game_mode = GAME_INTRO;
	sound_init();
	if (!gfxmap_init()) {
		return false;
	}
	if (!themes_load()) {
		return false;
	}
	if (!grid_maps_load()) {
		return false;
	}
	if (!themes_exists("linux")) {
		return false;
	}
	options_read();
	intro_init();
	menu_init();
	hiscore_read();
	snowflake_init();
	pause = false;
	return true;
}

void game_editor_start(void) {
	previous_game_mode = game_mode;
	game_mode = GAME_EDITOR;
	pause = false;
	editor_init();
}

void game_editor_stop(void) {
	if (!grid_maps_load()) {
		return;
	}
	hiscore_read();
	editor_close();
	game_mode = previous_game_mode;
	if (editor_current_grid_changed()) {
		options_write();
		if (game_mode == GAME_LEVEL || game_mode == GAME_INTERMISSION) {
			game_start();
			return;
		}
	}
	pause = true;
}

void game_start(void) {
	pause = false;
	intro_close();
	level_close();
	stats.score = 0;
	stats.level = 1;
	stats.dead = false;
	level_init(&stats);
	game_mode = GAME_LEVEL;
}

static void game_over(void) {
	if (!grid_maps_random(grid_maps_current_name()) && hiscore_check_high(grid_maps_current_name(), stats.score)) {
		hiscore_dialog_init();
		game_mode = GAME_HIGH_SCORE;
	} else {
		game_start();
	}
}

static void game_continue(void) {
	intro_close();
	level_close();
	stats.level++;
	level_init(&stats);
	game_mode = GAME_LEVEL;
}

static void game_intermission_start(void) {
	intermission_init(&stats);
	game_mode = GAME_INTERMISSION;
}

bool game_update(u32 keys, touchPosition touch) {
#ifdef DEBUG
	if (keys & KEY_X) {
		return false;
	}
#endif
	if ((keys & KEY_START) && game_mode != GAME_HIGH_SCORE && game_mode != GAME_EDITOR) {
		pause = !pause;
		if (pause) {
			menu_init();
		}
	}
	if (menu_update(pause, keys)) {
		return true;
	}
	switch(game_mode) {
	case GAME_INTRO:
		if (!intro_update(keys)) {
			game_start();
		}
		break;
	case GAME_LEVEL:
		if (!level_update(keys, touch, &stats)) {
			game_intermission_start();
		}
		break;
	case GAME_EDITOR:
		if (!editor_update(keys, touch)) {
			game_editor_stop();
		}
		break;
	case GAME_INTERMISSION:
		if (!intermission_update(keys)) {
			printf("GAME OVER: score=%ld base=%ld clearbonus=%d lifebonus=%d dead=%d\n", stats.score, stats.basescore, stats.clearbonus, stats.lifebonus, stats.dead);
			stats.score += stats.basescore + stats.clearbonus + stats.lifebonus;
			stats.basescore = 0;
			if (stats.dead) {
				game_over();
			} else {
				game_continue();
			}
		}
		break;
	case GAME_HIGH_SCORE:
		if (hiscore_dialog_update(keys, touch, &stats)) {
			game_start();
		}
		break;
	default:
		break;
	}
	return true;
}

void game_draw_bottom(C3D_RenderTarget *bottom) {
	C2D_TargetClear(bottom, C2D_Color32(0, 0, 0, 255));
	C2D_SceneBegin(bottom);
	switch(game_mode) {
	case GAME_INTRO:
		intro_draw();
		break;
	case GAME_EDITOR:
		editor_draw();
		return;
	case GAME_LEVEL:
		level_draw();
		break;
	case GAME_INTERMISSION:
		level_draw();
		intermission_draw();
		break;
	case GAME_HIGH_SCORE:
		level_draw();
		hiscore_dialog_draw();
		break;
	default:
		break;
	}
	menu_bottom_draw(pause);
	return;
}

void game_draw_top(C3D_RenderTarget *top, float parallax) {
	C2D_TargetClear(top, C2D_Color32(0, 0, 0, 255));
	C2D_SceneBegin(top);
	menu_draw(pause, &stats, parallax);
	return;
}

void game_close(void) {
	gfxmap_close();
	menu_close();
	sound_close();
}
