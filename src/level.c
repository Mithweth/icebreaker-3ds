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

/* special thanks to Enrico for the grid coloring stuff for themes */

#include <3ds.h>
#include <stdio.h>
#include <stdlib.h>
#include "game.h"
#include "line.h"
#include "grid.h"
#include "grid_maps.h"
#include "level.h"
#include "options.h"
#include "themes.h"
#include "sound.h"
#include "flying_sprite.h"
#include "penguin.h"
#include "text.h"
#include "lang.h"

#define RANDOM_HISTORY 			3
#define GESTURE_THRESHOLD 		12
#define FREEZE_DURATION_SECONDS	2.0f
#define SPEED_DURATION_SECONDS	1.0f
#define RESTORE_LINES_PCT		50
#define RESTORE_LINES_DELAY		7  // seconds

static Line line1;
static Line line2;
static int penguincount=0;
static Penguin flock[MAXPENGUINS];
static bool gesture_active = false;
static touchPosition gesture_start;
static touchPosition gesture_last;
static int timepenalty;
static int timepenaltyinterval;
static float scoremod;
static float bonusmod;
static bool done;
static int percentextrabonus;
static int percentbonus;
static int tick;
static size_t freeze_tick;
static size_t speed_tick;
static bool shield_enabled;
static bool shield_used;
static bool challenge_explained;
static int clear_pct_required;

typedef enum {
	CHALLENGE_CONQUEROR,
	CHALLENGE_LAST_CHANCE,
	CHALLENGE_TIME_LIMIT,
	CHALLENGE_NO_POWERUP,
	CHALLENGE_SPEEDY_PENGUINS,
	CHALLENGE_UNSTABLE_LINES,
	CHALLENGE_SLOW_LINES,
	CHALLENGE_COUNT,
} ChallengeMode;

typedef struct {
	const char* label;
	ChallengeMode mode;
} ChallengeEntry;

static const char* challenge_names[] = {
	"CHALLENGE_CONQUEROR",
	"CHALLENGE_LAST_CHANCE",
	"CHALLENGE_TIME_LIMIT",
	"CHALLENGE_NO_POWERUP",
	"CHALLENGE_SPEEDY_PENGUINS",
	"CHALLENGE_UNSTABLE_LINES",
	"CHALLENGE_SLOW_LINES"
};
static size_t current_challenge;
static size_t random_history_count;
static size_t random_history[RANDOM_HISTORY];
static bool unstable_lines;
static bool slow_lines;
static size_t unstable_lines_tick;


static bool challenge_in_history(size_t challenge) {
	for (size_t i = 0; i < random_history_count; i++) {
		if (random_history[i] == challenge) return true;
	}
	return false;
}

static void level_action_startclick(int mousex, int mousey, bool vertical)
{
	int x,y,xdif,ydif;

	x = mousex / BLOCKWIDTH * BLOCKWIDTH;
	y = mousey / BLOCKHEIGHT * BLOCKHEIGHT;
	xdif = mousex - x; ydif = mousey - y;

	if (grid_at(x, y) == '*' || grid_at(mousex, mousey) == '*') 
	{ // a little bit of grace if the player clicks directly on the penguin
		sound_play(SNDOUCH);
	}
	else if (grid_at(x, y) == ' ')
	{
		if (vertical)
		{
			if (!line1.on) {
				(ydif<BLOCKHEIGHT/2) ? line_start(&line1, UP, x, y, 0) : line_start(&line1, UP, x, y+BLOCKHEIGHT, 0);
				line_enable_shield(&line1, shield_enabled);
			}
			if (!line2.on) {
				(ydif<BLOCKHEIGHT/2) ? line_start(&line2, DOWN, x, y, 0) : line_start(&line2, DOWN, x, y+BLOCKHEIGHT, 0);
				line_enable_shield(&line2, shield_enabled);
			}
		} else {
			if (!line1.on) {
				(xdif<BLOCKWIDTH/2) ? line_start(&line1, LEFT, x, y, 0) : line_start(&line1, LEFT, x+BLOCKWIDTH,y, 0);
				line_enable_shield(&line1, shield_enabled);
			}
			if (!line2.on) {
				(xdif<BLOCKWIDTH/2) ? line_start(&line2, RIGHT, x, y, 0) : line_start(&line2, RIGHT, x+BLOCKWIDTH,y, 0);
				line_enable_shield(&line2, shield_enabled);
			}
		}
	}
}

static void level_rebuild_grid(void) {
	grid_restore_base();
	line_replay_history();
	if (line1.on) grid_mark(line1.geom.x, line1.geom.y, line1.geom.w, line1.geom.h, line1.id);
	if (line2.on) grid_mark(line2.geom.x, line2.geom.y, line2.geom.w, line2.geom.h, line2.id);

	for (int i = 0; i < penguincount; i++) {
		grid_mark(flock[i].state.x, flock[i].state.y, BLOCKWIDTH, BLOCKHEIGHT, '*');
	}
	grid_fill_empty_areas();
}

void level_challenge_pick(void) {
	challenge_explained = false;
	current_challenge = random() % CHALLENGE_COUNT;
	while (random_history_count < CHALLENGE_COUNT && challenge_in_history(current_challenge)) {
		current_challenge = random() % CHALLENGE_COUNT;
	}
	if (random_history_count == RANDOM_HISTORY) {
		for (size_t i = 1; i < random_history_count; i++) random_history[i - 1] = random_history[i];
		random_history_count--;
	}
	random_history[random_history_count++] = current_challenge;
}

void level_init(GameStats *stats) {
	line_reset();
	grid_generate_colors();
	grid_maps_apply(grid_maps_current_name());
	flying_sprite_init();
	scoremod = 1.0f;
	bonusmod = 1.0f;
	freeze_tick = 0;
	stats->level_tick = -1;
	speed_tick = 0;
	shield_enabled = false;
	shield_used = false;
	clear_pct_required = PERCENTREQUIRED;
	percentextrabonus = PERCENTEXTRABONUS - (stats->level - 1) / 5; // this needs to decrease, or else the bonus disintegrates as the space required to actually trap penguins increases
	percentbonus = PERCENTBONUS; // right now, there's really no point in making this a variable but it doesn't hurt, and keeps things consistant
	timepenalty = 0;
	done = false;
	penguincount = stats->level + 1;
	if (penguincount > MAXPENGUINS) {
		penguincount = MAXPENGUINS;
	}
	float speed_modifier = 1.0f;
	stats->lives = stats->level + 1;
	unstable_lines = false;
	slow_lines = false;
	unstable_lines_tick = 0;
	if (options_challenge_mode()) {
		switch(current_challenge) {
		case CHALLENGE_LAST_CHANCE:
			stats->lives = 1;
			break;
		case CHALLENGE_SPEEDY_PENGUINS:
			speed_modifier = 2.0f;
			break;
		case CHALLENGE_CONQUEROR:
			clear_pct_required = PERCENTREQUIREDCHALLENGE;
			break;
		case CHALLENGE_NO_POWERUP:
			stats->previous_energy = stats->energy;
			stats->energy = 0;
			break;
		case CHALLENGE_TIME_LIMIT:
			stats->level_tick = 60 * FPS;
			break;
		case CHALLENGE_UNSTABLE_LINES:
			unstable_lines = true;
			break;
		case CHALLENGE_SLOW_LINES:
			slow_lines = true;
			break;
		default:
			break;
		}
	}
	switch(options_difficulty_current())
	{
		case NORMAL:
			scoremod=(stats->level+1)/2.0f;
			bonusmod=(stats->level+1)/2.0f;
			timepenaltyinterval=100;
		break;
		case EASY:
			scoremod=(stats->level+2)/6.0f;
			bonusmod=(stats->level+1)/9.0f;
			timepenaltyinterval=200;
		break;
		case HARD:
			scoremod=(stats->level+1)/1.75f;
			bonusmod=(stats->level+1)/1.5f;
			timepenaltyinterval=75;
		break;
		default:
			fprintf(stderr,"Unknown difficulty -- that can't happen!\n");
			return;
		break;
	}
	stats->basescore = 0;
	stats->clearbonus = 0;
	stats->lifebonus = 0;
	stats->energybonus = 0;
	stats->clear = 0;
	line1=line_create(1);
	line2=line_create(2);

	for (int i=0;i<penguincount;i++)
	{
		flock[i] = penguin_create();
		penguin_speed(&flock[i], speed_modifier);
		if (options_modern_mode()) {
			flock[i].slow_axis = random() % SLOW_AXIS_COUNT;
		}
	}
	tick = 0;
}

void level_draw(void) {
	grid_drawblocks();
	if (line1.on) {
		line_draw(&line1, theme.color.line1);
	}
	if (line2.on) {
		line_draw(&line2, theme.color.line2);
	}
	for (int i=0;i<penguincount;i++) {
		penguin_draw(&flock[i]);
	}
	if (!challenge_explained && options_challenge_mode()) {
		C2D_DrawRectSolid(0, 0, 0.45f, WIDTH, HEIGHT, C2D_Color32(0, 0, 0, 192));
		text_cput(160.0f, 80.0f, 0.46f, 3, theme.color.gameovertext, lang_get(challenge_names[current_challenge]));
		char explain[255];
		snprintf(explain, sizeof(explain), "%s_EXPLAIN_1", challenge_names[current_challenge]);
		text_cput(160.0f, 120.0f, 0.46f, 3, theme.color.scorescrolltext, lang_get(explain));
		snprintf(explain, sizeof(explain), "%s_EXPLAIN_2", challenge_names[current_challenge]);
		text_cput(160.0f, 140.0f, 0.46f, 3, theme.color.scorescrolltext, lang_get(explain));
		return;
	}
	if (freeze_tick > 0) {
		size_t max_freeze_tick = (size_t)(FREEZE_DURATION_SECONDS * FPS);
		if (freeze_tick > max_freeze_tick - 5) {
			C2D_DrawRectSolid(0, 0, 0.45f, WIDTH, HEIGHT, C2D_Color32(255, 255, 255, 255));
		} else {
			C2D_DrawRectSolid(0, 0, 0.45f, WIDTH, HEIGHT, C2D_Color32(255, 255, 255, 128 * freeze_tick / max_freeze_tick));		
		}
	} else if (speed_tick > 0) {
		size_t max_speed_tick = (size_t)(SPEED_DURATION_SECONDS * FPS);
		if (speed_tick > max_speed_tick - 5) {
			C2D_DrawRectSolid(0, 0, 0.45f, WIDTH, HEIGHT, C2D_Color32(255, 0, 0, 255));
		} else {
			C2D_DrawRectSolid(0, 0, 0.45f, WIDTH, HEIGHT, C2D_Color32(255, 0, 0, 128 * speed_tick / max_speed_tick));		
		}
	} else if (shield_enabled) {
		C2D_DrawRectSolid(0, 0, 0.45f, WIDTH, HEIGHT, theme.color.shieldbackground);
		C2D_ImageTint tint;
		C2D_AlphaImageTint(&tint, 0.15f);
		C2D_DrawImageAt(theme.shieldbitmap, 160.0f - theme.shieldbitmap.subtex->width / 2, 120.0f - theme.shieldbitmap.subtex->height / 2, 0.46f, &tint, 1.0f, 1.0f);
	}
}

bool level_update(u32 keys, touchPosition touch, GameStats *stats) {
	if (done) {
		return false;
	}
	if (!challenge_explained && options_challenge_mode()) {
		if (keys) challenge_explained = true;
		return true;
	}
	if (stats->level_tick > 0) {
		stats->level_tick--;
		if (stats->level_tick == 0) {
			stats->lives = 0;
		}
	}
	if (unstable_lines) {
		if (unstable_lines_tick++ > RESTORE_LINES_DELAY * FPS) {
			if (random() % RESTORE_LINES_PCT == 0) {
				unstable_lines_tick = 0;
				long count_before = grid_countcleared();
				long count_after = count_before;
				while (count_after == count_before && line_remove()) {
					level_rebuild_grid();
					count_after = grid_countcleared();
				}
				stats->clear = count_after;
				stats->basescore = (int)(stats->clear * scoremod) - timepenalty;
			}
		}
	}
	int line_speed = 1;
	if (slow_lines) {
		line_speed = tick % 2;
	}
	if (shield_used && !line1.on && !line2.on) {
		shield_enabled = false;
		shield_used = false;
		sound_play(SNDSHIELDOFF);
	}
	if (options_modern_mode() && stats->energy > 0  && speed_tick + freeze_tick == 0) {
		if ((keys & (KEY_DLEFT | KEY_Y)) && stats->energy > 1 && !shield_enabled) { // Shield
			stats->energy -= 2;
			shield_enabled = true;
			if (line1.on) line_enable_shield(&line1, true);
			if (line2.on) line_enable_shield(&line2, true);
			sound_play(SNDSHIELD);
		} else if (keys & (KEY_DUP | KEY_X)) { // Freeze
			stats->energy--;
			freeze_tick = (size_t)(FREEZE_DURATION_SECONDS * FPS);
			sound_play(SNDFREEZE);
		} else if (keys & (KEY_DDOWN | KEY_B)) { // Speed
			stats->energy--;
			speed_tick = (size_t)(SPEED_DURATION_SECONDS * FPS);
			sound_play(SNDSPEED);
		}
	}

	if (keys & KEY_TOUCH) {
		gesture_start = touch;
		gesture_last = touch;
		gesture_active = true;
		flying_sprite_watch();
	}

	if (gesture_active && (hidKeysHeld() & KEY_TOUCH)) {
		gesture_last = touch;
	}

	if (gesture_active && (hidKeysUp() & KEY_TOUCH)) {
		int dx = (int)gesture_last.px - (int)gesture_start.px;
		int dy = (int)gesture_last.py - (int)gesture_start.py;

		if (abs(dx) >= GESTURE_THRESHOLD || abs(dy) >= GESTURE_THRESHOLD) {
			bool vertical = abs(dy) > abs(dx);

			level_action_startclick(gesture_start.px, gesture_start.py, vertical);
		}
		gesture_active = false;
	}

	if (freeze_tick > 0) freeze_tick--;
	if (speed_tick > 0) speed_tick--;
	bool life_lost = false;
	int speed = (speed_tick > 0) ? 3 : line_speed;
	for(int i = 0; i < speed; i++) {
		if (line1.on) {
			// a line hit by a penguin last frame must not move, or it
			// could finish and fill its area before being killed
			if (!line1.dead) {
				// kludgy crap to implement speed = 1 1/2
				bool linedoneflag = line_move(&line1);
				if (!linedoneflag && line1.speedslower) {
					line1.speedslower = false;
					linedoneflag = line_move(&line1);
				} else {
					line1.speedslower = true;
				}
				if (linedoneflag) {
					flying_sprite_capture();
					stats->clear = grid_countcleared();
					stats->basescore = (int)(stats->clear * scoremod) - timepenalty;
					tick = 0; // reset this to keep score from seeming to flicker around a lot. and to be nice. :)
				}
			}
			if (line1.dead)  {
				line_kill(&line1);
				life_lost = true;
			}
		}
		if (line2.on) {
			if (!line2.dead) {
				bool linedoneflag = line_move(&line2);
				if (!linedoneflag && line2.speedslower) {
					line2.speedslower = false;
					linedoneflag = line_move(&line2);
				} else {
					line2.speedslower = true;
				}
				if (linedoneflag) {
					flying_sprite_capture();
					stats->clear = grid_countcleared();
					stats->basescore = (int)(stats->clear * scoremod) - timepenalty;
					tick = 0; // reset this to keep score from seeming to flicker around a lot. and to be nice. :)
				}
			}
			if (line2.dead) {
				line_kill(&line2);
				life_lost = true;
			}
		}
	}
	if (life_lost) {
		sound_play(SNDBREAK);
		flying_sprite_death();
		stats->lives--;
		if (stats->lives < 0) stats->lives = 0;
	}
	if (line1.shield_hit || line2.shield_hit) shield_used = true;
	for (int i = 0; i < penguincount; i++)
	{
		PenguinHit hit = penguin_move(&flock[i], stats->level, &line1, &line2, freeze_tick > 0);
		if (hit == PENGUIN_HIT_LINE1 || hit == PENGUIN_HIT_BOTH) line1.dead = true;
		if (hit == PENGUIN_HIT_LINE2 || hit == PENGUIN_HIT_BOTH) line2.dead = true;
		if (hit == PENGUIN_HIT_SHIELD) shield_used = true;
	}

	if (stats->lives <= 0) // game over
	{
		stats->dead = true;
		speed_tick = 0;
		freeze_tick = 0;
		shield_enabled = false;
		flying_sprite_gameover();
		done = true;
		return true;
	}
	if (!line1.on && !line2.on && stats->clear >= clear_pct_required) // success!
	{ 
		flying_sprite_victory();
		speed_tick = 0;
		freeze_tick = 0;
		shield_enabled = false;
		done = true;
		stats->level_tick = -1;
		stats->basescore=(int)(stats->clear * scoremod) - timepenalty;

		stats->clearbonus=0;
		// bonuses for clearing lots
		if (stats->clear > percentbonus) {
			stats->clearbonus += (int)((stats->clear-percentbonus) * bonusmod);
		}
		if (stats->clear > percentextrabonus) {
			stats->clearbonus += (int)((stats->clear - percentextrabonus) * (stats->clear - percentextrabonus) * bonusmod);
			stats->energybonus += 1;
		}
		if ((options_challenge_mode() && current_challenge == CHALLENGE_LAST_CHANCE) || stats->lives == stats->level + 1) {
			stats->energybonus += 1;
		}
			
		// bonuses for leftover stats->lives
		stats->lifebonus = (int)((stats->lives - 1) * 3 * bonusmod);
	}

	tick++;
	if (tick >= timepenaltyinterval * FPS / 100) {
		tick = 0;
		if (stats->basescore>0) {
			timepenalty++;
			stats->basescore=(int)(stats->clear*scoremod)-timepenalty;
		}
	}
	return true;
}

void level_close(void) {
	return;
}
