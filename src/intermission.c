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

static char first_line_text[30];
static char second_line_text[30];
static float first_line_x;
static float first_line_y;
static float second_line_x;
static float second_line_y;
static bool done;

void intermission_init(const GameStats *stats) {
	if (!stats->dead) {
		snprintf(first_line_text,30,"SCORE: %ld",stats->basescore);
		snprintf(second_line_text,30, "BONUS: %d",stats->clearbonus + stats->lifebonus);
		sound_play(SNDVICTORY);
	} else {
		snprintf(first_line_text,30,"FINAL SCORE: %ld",stats->basescore + stats->score);
		snprintf(second_line_text,30, "GAME OVER");
	}
	int first_line_w = text_getwidth(4,first_line_text);
	int second_line_w = text_getwidth(4,second_line_text);

	first_line_x=(WIDTH - first_line_w) / 2;
	first_line_y=HEIGHT;

	second_line_x=(WIDTH - second_line_w) / 2;
	second_line_y=HEIGHT;
	done = false;
}

void intermission_draw(void) {
	C2D_DrawRectSolid(BORDERLEFT, BORDERTOP, 0.4f, PLAYWIDTH, PLAYHEIGHT, C2D_Color32(0, 0, 0, 192));
	text_put(first_line_x, first_line_y, 0.5f, 4, theme.color.scorescrolltext, first_line_text);
	text_put(second_line_x, second_line_y, 0.5f, 4, theme.color.bonusscrolltext, second_line_text);
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
