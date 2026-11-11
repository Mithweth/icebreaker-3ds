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
#include "logo.h"
#include "mesh.h"
#include "transition.h"

typedef enum {
	GAME_INTRO,
	GAME_LEVEL,
	GAME_HIGH_SCORE,
	GAME_EDITOR,
	GAME_INTERMISSION,
	GAME_OVER
} GameStep;

static GameStep game_step;
static GameStep previous_game_step;

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
	game_step = GAME_INTRO;
	sound_init();
	mesh_init();
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
	logo_init();
	menu_init();
	hiscore_read();
	snowflake_init();
	hiscore_dialog_init();
	pause = false;
	return true;
}

void game_editor_start(void) {
	previous_game_step = game_step;
	game_step = GAME_EDITOR;
	pause = false;
	editor_init();
}

void game_editor_stop(void) {
	if (!grid_maps_load()) {
		return;
	}
	hiscore_read();
	editor_close();
	game_step = previous_game_step;
	if (editor_current_grid_changed()) {
		options_write();
		if (game_step == GAME_LEVEL || game_step == GAME_INTERMISSION) {
			game_start();
			return;
		}
	}
	pause = true;
}

void game_start(void) {
	transition_close();
	pause = false;
	transition_init((game_step == GAME_INTRO) ? TRANSITION_FLIP : TRANSITION_SPLIT);
	intro_close();
	level_close();
	stats.score = 0;
	stats.level = 1;
	stats.previous_energy = 0;
	stats.energy = 1;
	stats.dead = false;
	stats.drawn_lines = 0;
	stats.max_captured_zone = 0;
	stats.total_time_tick = 0;
	stats.powerup_used = 0;
	stats.total_energy = 1;
	stats.captured_blocks = 0;
	stats.max_safe_line_count = 0;
	stats.safe_line_count = 0;
	stats.no_lost_life_level = 0;
	GameStats saved_stats = stats;
	while (!level_init(&stats)) {
		stats = saved_stats;
	}
	game_step = GAME_LEVEL;
}

static void game_over(void) {
	if (!grid_maps_random(grid_maps_current_name()) && !options_challenge_mode() && hiscore_check_high(options_game_mode_current(), grid_maps_current_name(), stats.score)) {
		game_step = GAME_HIGH_SCORE;
	} else {
		game_start();
	}
}

static void game_continue(void) {
	level_close();
	stats.level++;
	if (stats.previous_energy > 0) {
		stats.energy = stats.previous_energy;
		stats.previous_energy = 0;
	}
	stats.energy++;
	stats.energy += stats.energybonus;
	stats.total_energy += stats.energybonus + 1;
	GameStats saved_stats = stats;
	while (!level_init(&stats)) {
		stats = saved_stats;
	}
	game_step = GAME_LEVEL;

	transition_init(TRANSITION_EXPLODE_IN);
}

static void game_intermission_start(void) {
	transition_init(TRANSITION_EXPLODE_OUT);
	intermission_init(&stats);
	game_step = GAME_INTERMISSION;
}

bool game_update(u32 keys, touchPosition touch) {
#ifdef DEBUG
	if (keys & (KEY_L | KEY_R)) {
		return false;
	}
#endif
	if ((keys & KEY_START) && game_step != GAME_HIGH_SCORE && game_step != GAME_EDITOR) {
		pause = !pause;
		if (pause) {
			menu_init();
		}
	}
	if (menu_update(pause, keys)) {
		return true;
	}

	if (transition_active()) {
		if (transition_update()) return true;
		transition_close();
	}
	switch(game_step) {
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
	switch(game_step) {
	case GAME_INTRO:
		intro_draw();
		break;
	case GAME_EDITOR:
		editor_draw();
		return;
	case GAME_LEVEL:
		if (!transition_hides_screen()) level_draw(bottom);
		break;
	case GAME_INTERMISSION:
		intermission_draw();
		break;
	case GAME_HIGH_SCORE:
		level_draw(bottom);
		hiscore_dialog_draw();
		break;
	default:
		break;
	}
	transition_draw(bottom);
	menu_bottom_draw(pause);
	return;
}

void game_draw_top(C3D_RenderTarget *target, float parallax) {
	menu_draw(pause, &stats, target, parallax);
}

void game_close(void) {
	gfxmap_close();
	level_close();
	logo_close();
	menu_close();
	mesh_close();
	sound_close();
}
