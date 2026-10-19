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
#include "game.h"
#include "themes.h"
#include "text.h"
#include "sound.h"
#include "lang.h"

static char first_line_text[30];
static char second_line_text[30];
static float first_line_x;
static float first_line_y;
static float second_line_x;
static float second_line_y;
static bool done;
static size_t first_line_size;
static size_t second_line_size;
static u32 second_line_color;

void intermission_init(const GameStats *stats) {
	if (!stats->dead) {
		snprintf(first_line_text,30,"%s: %ld", lang_get("INTERMISSION_SCORE"), stats->basescore);
		snprintf(second_line_text,30, "%s: %d", lang_get("INTERMISSION_BONUS"), stats->clearbonus + stats->lifebonus);
		second_line_color = theme.color.bonusscrolltext;
		sound_play(SNDVICTORY);
	} else {
		snprintf(first_line_text,30,"%s: %ld", lang_get("INTERMISSION_FINAL_SCORE"), stats->basescore + stats->score);
		snprintf(second_line_text,30, "%s", lang_get("INTERMISSION_GAME_OVER"));
		second_line_color = theme.color.gameovertext;
		sound_play(SNDDEFEAT);
	}
	int first_line_w;
	int second_line_w;
	for(size_t size = 4; size > 0; size--) {
		first_line_w = text_getwidth(size, first_line_text);
		if (first_line_w < WIDTH) {
			first_line_size = size;
			break;
		}
	}
	for(size_t size = 4; size > 0; size--) {
		second_line_w = text_getwidth(size, second_line_text);
		if (second_line_w < WIDTH) {
			second_line_size = size;
			break;
		}
	}

	first_line_x = (WIDTH - first_line_w) / 2;
	first_line_y = HEIGHT;

	second_line_x = (WIDTH - second_line_w) / 2;
	second_line_y = HEIGHT;
	done = false;
}

void intermission_draw(void) {
	C2D_DrawRectSolid(0, 0, 0.4f, WIDTH, HEIGHT, C2D_Color32(0, 0, 0, 192));
	text_put(first_line_x, first_line_y, 0.5f, first_line_size, theme.color.scorescrolltext, first_line_text);
	text_put(second_line_x, second_line_y, 0.5f, second_line_size, second_line_color, second_line_text);
}

bool intermission_update(u32 keys) {
	if (keys & KEY_TOUCH) {
		done = true;
	}
	if (done) return false;
	if (first_line_y > 100) {
		first_line_y-=2;
	} else if (second_line_y > 130) {
		second_line_y-=2;
	}
	return true;
}
