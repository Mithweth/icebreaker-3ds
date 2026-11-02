/*
 * IceBreaker
 * Copyright (c) 2000-2020 Matthew Miller <mattdm@mattdm.org>
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
#include "game.h"
#include "themes.h"
#include "text.h"
#include "sound.h"
#include "lang.h"
#include "options.h"

static char first_line_text[30];
static float first_line_x;
static float first_line_y;
static size_t first_line_size;
static char second_line_text[30];
static float second_line_x;
static float second_line_y;
static size_t second_line_size;
static u32 second_line_color;
static char third_line_text[30];
static float third_line_x;
static float third_line_y;
static size_t third_line_size;
static bool done;
static bool show_stats;
static const GameStats *current_stats;

void intermission_init(const GameStats *stats) {
	current_stats = stats;
	if (!stats->dead) {
		snprintf(first_line_text,30,"%s: %ld", lang_get("INTERMISSION_SCORE"), stats->basescore);
		snprintf(second_line_text,30, "%s: %d", lang_get("INTERMISSION_BONUS"), stats->clearbonus + stats->lifebonus);
		snprintf(third_line_text,30, "%s: %zu", lang_get("INTERMISSION_ENERGY"), stats->energybonus);
		second_line_color = theme.color.bonusscrolltext;
		sound_play(SNDVICTORY);
	} else {
		snprintf(first_line_text,30,"%s: %ld", lang_get("INTERMISSION_FINAL_SCORE"), stats->basescore + stats->score);
		snprintf(second_line_text,30, "%s", lang_get("INTERMISSION_GAME_OVER"));
		second_line_color = theme.color.gameovertext;
		sound_play(SNDDEFEAT);
	}
	int first_line_w;
	for(size_t size = 4; size > 0; size--) {
		first_line_w = text_getwidth(size, first_line_text);
		if (first_line_w < WIDTH) {
			first_line_size = size;
			break;
		}
	}
	int second_line_w;
	for(size_t size = 4; size > 0; size--) {
		second_line_w = text_getwidth(size, second_line_text);
		if (second_line_w < WIDTH) {
			second_line_size = size;
			break;
		}
	}
	int third_line_w;
	for(size_t size = 4; size > 0; size--) {
		third_line_w = text_getwidth(size, third_line_text);
		if (third_line_w < WIDTH) {
			third_line_size = size;
			break;
		}
	}

	first_line_x = (WIDTH - first_line_w) / 2;
	first_line_y = HEIGHT;

	second_line_x = (WIDTH - second_line_w) / 2;
	second_line_y = HEIGHT;

	third_line_x = (WIDTH - third_line_w) / 2;
	third_line_y = HEIGHT;
	show_stats = false;
	done = false;
}

void intermission_draw(void) {
	C2D_DrawRectSolid(0, 0, 0.5f, WIDTH, HEIGHT, C2D_Color32(0, 0, 0, 192));
	if (!show_stats) {
		text_put(first_line_x, first_line_y, 0.51f, first_line_size, theme.color.scorescrolltext, first_line_text);
		text_put(second_line_x, second_line_y, 0.51f, second_line_size, second_line_color, second_line_text);
		if (options_modern_mode() && current_stats->energybonus > 0 && !current_stats->dead) {
			text_put(third_line_x, third_line_y, 0.51f, third_line_size, theme.color.bonusscrolltext, third_line_text);
		}
		return;
	}
	char tmptext[128];
	text_cput(160, 20, 0.51f, 3, theme.color.bonusscrolltext, lang_get("STATS_TITLE"));
	snprintf(tmptext, sizeof(tmptext), "%zu", current_stats->drawn_lines);
	text_put(10, 70, 0.51f, 2, theme.color.normaltext, lang_get("STATS_LINES_DRAWN"));
	text_rput(310, 70, 0.51f, 2, theme.color.normaltext, tmptext);
	snprintf(tmptext, sizeof(tmptext), "%ld", current_stats->max_captured_zone);
	text_put(10, 90, 0.51f, 2, theme.color.normaltext, lang_get("STATS_MAX_CAPTURED_ZONE"));
	text_rput(310, 90, 0.51f, 2, theme.color.normaltext, tmptext);
	snprintf(tmptext, sizeof(tmptext), "%zu", current_stats->total_time_tick / FPS / current_stats->level);
	text_put(10, 110, 0.51f, 2, theme.color.normaltext, lang_get("STATS_AVERAGE_LEVEL_TIME"));
	text_rput(310, 110, 0.51f, 2, theme.color.normaltext, tmptext);
	snprintf(tmptext, sizeof(tmptext), "%zu", current_stats->max_safe_line_count);
	text_put(10, 130, 0.51f, 2, theme.color.normaltext, lang_get("STATS_MAX_CONSECUTIVE_LINES"));
	text_rput(310, 130, 0.51f, 2, theme.color.normaltext, tmptext);
	long average = (current_stats->drawn_lines > 0) ? current_stats->captured_blocks / current_stats->drawn_lines : 0;
	snprintf(tmptext, sizeof(tmptext), "%ld", average);
	text_put(10, 150, 0.51f, 2, theme.color.normaltext, lang_get("STATS_AVERAGE_BLOCKS_PER_LINE"));
	text_rput(310, 150, 0.51f, 2, theme.color.normaltext, tmptext);
	snprintf(tmptext, sizeof(tmptext), "%zu", current_stats->no_lost_life_level);
	text_put(10, 170, 0.51f, 2, theme.color.normaltext, lang_get("STATS_LEVELS_WITHOUT_LIFE_LOSS"));
	text_rput(310, 170, 0.51f, 2, theme.color.normaltext, tmptext);
	if (options_modern_mode()) {
		snprintf(tmptext, sizeof(tmptext), "%zu", current_stats->powerup_used);
		text_put(10, 190, 0.51f, 2, theme.color.normaltext, lang_get("STATS_POWERUPS_USED"));
		text_rput(310, 190, 0.51f, 2, theme.color.normaltext, tmptext);
		snprintf(tmptext, sizeof(tmptext), "%zu", current_stats->total_energy);
		text_put(10, 210, 0.51f, 2, theme.color.normaltext, lang_get("STATS_ENERGY_GAINED"));
		text_rput(310, 210, 0.51f, 2, theme.color.normaltext, tmptext);
	}
}

bool intermission_update(u32 keys) {
	if (keys & KEY_TOUCH) {
		if (current_stats->dead && !show_stats) {
			show_stats = true;
		} else {
			done = true;
		}
	}
	if (done) return false;
	if (first_line_y > 100) {
		first_line_y -= 2;
	} else if (second_line_y > 130) {
		second_line_y -= 2;
	} else if (third_line_y > 160) {
		third_line_y -= 2;
	}
	return true;
}
