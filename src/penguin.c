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

#include <citro2d.h>
#include <stdlib.h>
#include "game.h"
#include "penguin.h"
#include "themes.h"
#include "grid.h"
#include "options.h"
#include "line.h"
#include "level.h"

#define CHARGE_TICKS_SECONDS    0.5f
#define ANGLE_PHASES			3

static char penguin_edge(int x0, int y0, int x1, int y1) {
	char found = ' ';
	int steps = ((x1 - x0) + (y1 - y0)) / BLOCKWIDTH + 1;
	for (int s = 0; s <= steps; s++) {
		int x = x0 + (x1 - x0) * s / steps;
		int y = y0 + (y1 - y0) * s / steps;
		char c = grid_at(x, y);
		if (c == '1' || c == '2') return c;  // a line in progress wins
		if (c != ' ') found = c;
	}
	return found;
}

// Since the play field is not square anymore, checking only the block
// first pixel is not enough. We need to ensure the penguin to hold
// fully on the block.
static bool penguin_occupy_block(int x, int y, int size) {
	const int w = BLOCKWIDTH * size;
	const int h = BLOCKHEIGHT * size;
	for (int dy = 0; dy < h; dy += BLOCKHEIGHT) {
		if (penguin_edge(x, y + dy, x + w - 1, y + dy) != ' ') return false;
	}
	return penguin_edge(x, y + h - 1, x + w - 1, y + h - 1) == ' ';
}

Penguin penguin_create(int size) {
	int x = random() % (WIDTH - BLOCKWIDTH * size);
	int y = random() % (HEIGHT - BLOCKHEIGHT * size);
	while(!penguin_occupy_block(x, y, size)) {
		x = random() % (WIDTH - BLOCKWIDTH * size);
		y = random() % (HEIGHT - BLOCKHEIGHT * size);
	}
	return penguin_createxy(x, y, size);
}

void penguin_teleport(Penguin* p) {
	grid_mark(p->state.x, p->state.y, BLOCKWIDTH*p->size, BLOCKHEIGHT*p->size, ' ');
	int x = random() % (WIDTH - BLOCKWIDTH*p->size);
	int y = random() % (HEIGHT - BLOCKHEIGHT*p->size);
	int tries = 0;
	while(!penguin_occupy_block(x, y, p->size) && ++tries < 100) {
		x = random() % (WIDTH - BLOCKWIDTH*p->size);
		y = random() % (HEIGHT - BLOCKHEIGHT*p->size);
	}
	if (tries == 100) {	// nowhere to go: stay put
		x = p->state.x;
		y = p->state.y;
	}
	grid_mark(x, y, BLOCKWIDTH*p->size, BLOCKHEIGHT*p->size, '*');
	grid_checkempty(p->state.x, p->state.y);
	p->state.x = x;
	p->state.y = y;
	p->history_count = 0;
}

Penguin penguin_createxy(int x, int y, int size)
{
	Penguin p;
	p.current_image = 0;
	p.image_tick = 0;
	switch (random() % 4)
	{
		case 0:
			p.state.xdelta= PENGUINSPEED;p.state.ydelta= PENGUINSPEED;
		break;
		case 1:
			p.state.xdelta= PENGUINSPEED;p.state.ydelta=-PENGUINSPEED;
		break;
		case 2:
			p.state.xdelta=-PENGUINSPEED;p.state.ydelta= PENGUINSPEED;
		break;
		case 3:
			p.state.xdelta=-PENGUINSPEED;p.state.ydelta=-PENGUINSPEED;
		break;
		default: // this won't happen, but it shuts up the warning.
			p.state.xdelta=-PENGUINSPEED;p.state.ydelta=-PENGUINSPEED;
		break;
	}
	
	// For debugging only, of course -- this makes crippled penguins.
	//p.state.xdelta=0; p.state.ydelta=0;
	
	p.state.speedslower=0;
	p.state.x = x;
	p.state.y = y;
	p.state.phase = 0;
	p.slow_axis = SLOW_NONE;
	p.history_count = 0;
	p.charge_tick = 0;
	p.teleport_tick = 0;
	p.wall_tick = 0;
	p.size = size;
	return p;
}

void penguin_speed(Penguin* p, float speed) {
	p->state.xdelta *= speed;
	p->state.ydelta *= speed;
}

void penguin_draw(Penguin* p)
{
	C2D_ImageTint tint;
	const C2D_ImageTint *t = NULL;
	if (options_modern_mode() && p->charge_tick > 0) {
		C2D_PlainImageTint(&tint, theme.color.spritecharge, 0.5f);
		t = &tint;
	}
	float scale_x = (p->state.xdelta < 0) ? -SPRITE_SIZE : SPRITE_SIZE;
	C2D_DrawImageAt(theme.spriteimage[p->current_image], p->state.x, p->state.y, 0.3f, t, scale_x * p->size, SPRITE_SIZE * p->size);
	if (p->teleport_tick > 0) {
		size_t teleport_image_index = (p->teleport_tick / (FPS / 6)) % theme.teleport_sprite_count;
		C2D_DrawImageAt(theme.teleportbitmap[teleport_image_index], p->state.x, p->state.y, 0.31f, t, scale_x * p->size, SPRITE_SIZE * p->size);
	}
}

PenguinHit penguin_move(Penguin* p, int level, const Line* line1, const Line* line2, bool freezed)
{
	if (freezed) return PENGUIN_HIT_NONE;
	const Line *lines[] = { line1, line2 };
	bool hitline1 = false;
	bool hitline2 = false;
	bool bounced_x = false;
	bool bounced_y = false;
	int newx, newy;
	int checkx, checky;
	int movex = 0;
	int movey = 0;
	int attract_radius = (level - 1) * 10;
	int speed_mod = 1;
	bool shield_touched = false;

	// Update the sprite in update since with stereoscopic 3d,
	// flying_sprite_draw() is called twice a frame
	if (p->image_tick++ >= 0.3f * FPS) { // 20 seems to be a correct value
		p->image_tick = 0;
		p->current_image = (p->current_image + 1) % theme.sprite_count;
	}

	if (p->charge_tick > 0) {
		p->charge_tick--;
	}

	switch (options_difficulty_current())
	{
		case NORMAL:
			if (p->state.speedslower) {
				movex = p->state.xdelta / 2;
				movey = p->state.ydelta / 2;
				if (p->charge_tick > 0) speed_mod = 2;
			} else {
				movex=p->state.xdelta;
				movey=p->state.ydelta;
			}
			p->state.speedslower = !p->state.speedslower;
			attract_radius += 90;
		break;
		case HARD:
			movex = p->state.xdelta;
			movey = p->state.ydelta;
			if (p->charge_tick > 0) speed_mod = 2;
			attract_radius += 120;
		break;
		case EASY:
			movex = p->state.xdelta / 2;
			movey = p->state.ydelta / 2;
			if (p->charge_tick > 0) speed_mod = 1 + (p->charge_tick % 2);
			attract_radius += 60;
		break;
	}

	if (attract_radius > 200) attract_radius = 200;
	movex *= speed_mod;
	movey *= speed_mod;

	if (p->slow_axis != SLOW_NONE) {
		p->state.phase = (p->state.phase + 1) % ANGLE_PHASES;
		if (p->state.phase == 0) {	// Don't move X or Y which impacts the angle
			if (p->slow_axis == SLOW_X) {
				movex = 0;
			} else {
				movey = 0;
			}
		}
	}

	newx=p->state.x+movex;
	newy=p->state.y+movey;

	grid_mark(p->state.x, p->state.y, BLOCKWIDTH * p->size, BLOCKHEIGHT * p->size, ' ');

	if (movex > 0) {
		checkx = newx + BLOCKWIDTH * p->size;
	} else {
		checkx = newx;
	}
	char edge = penguin_edge(checkx, p->state.y, checkx, p->state.y + BLOCKHEIGHT * p->size - 1);
	if (edge == ' ') {
		p->state.x += movex;
	} else if (edge == '1') {
		if (line1 && line_shield_enabled(line1)) {
			shield_touched = true;
			p->state.xdelta = -p->state.xdelta;
			bounced_x = true;
		} else {
			hitline1 = true;
			p->state.x += movex;
		}
	} else if (edge == '2') {
		if (line2 && line_shield_enabled(line2)) {
			shield_touched = true;
			p->state.xdelta = -p->state.xdelta;
			bounced_x = true;
		} else {
			hitline2 = true;
			p->state.x += movex;
		}
	} else {
		p->state.xdelta = -p->state.xdelta;
		bounced_x = true;
	}

	if (movey > 0) {
		checky = newy + BLOCKHEIGHT * p->size;
	} else {
		checky = newy;
	}
	edge = penguin_edge(p->state.x, checky, p->state.x + BLOCKWIDTH * p->size-1, checky);
	if (edge == ' ') {
		p->state.y += movey;
	} else if (edge == '1') {
		if (line1 && line_shield_enabled(line1)) {
			shield_touched = true;
			p->state.ydelta = -p->state.ydelta;
			bounced_y = true;
		} else {
			hitline1 = true;
			p->state.y += movey;
		}
	} else if (edge == '2') {
		if (line2 && line_shield_enabled(line2)) {
			shield_touched = true;
			p->state.ydelta = -p->state.ydelta;
			bounced_y = true;
		} else {
			hitline2 = true;
			p->state.y += movey;
		}
	} else {
		p->state.ydelta = -p->state.ydelta;
		bounced_y = true;
	}

	grid_mark(p->state.x,p->state.y,BLOCKWIDTH*p->size,BLOCKHEIGHT*p->size,'*');

	if (bounced_x || bounced_y) {
		if (options_modern_mode() && ((line1 && line1->on) || (line2 && line2->on))) {
			int best_dx = attract_radius;
			int best_dy = attract_radius;
			int px = p->state.x + BLOCKWIDTH * p->size / 2;
			int py = p->state.y + BLOCKHEIGHT * p->size / 2;
			for (int i = 0; i < 2; i++) {
				const Line *l = lines[i];
				if (!l || !l->on || l->dead) continue;
				int dx = l->mark.x - px;
				int dy = l->mark.y - py;

				if (dx * dx + dy * dy > attract_radius * attract_radius) continue;

				// No need to charge if the line cannot be seen.
				// So, if there's a hole between the penguin and the line
				// it is considered as not visible.
				bool visible = true;
				for (int s = 1; s < 10; s++) {
					if (!visible) break;
					char c = grid_at(px + dx * s / 10, py + dy * s / 10);
					if (c != ' ' && c != '*' && c != '1' && c != '2') visible = false;
				}
				if (!visible) continue;

				if (bounced_y && dx * dx < best_dx * best_dx) {
					best_dx = dx;
				}
				if (bounced_x && dy * dy < best_dy * best_dy) {
					best_dy = dy;
				}
			}
			if (p->charge_tick == 0 && bounced_y && best_dx != attract_radius) {
				if ((best_dx > 0 && p->state.xdelta < 0) || (best_dx < 0 && p->state.xdelta > 0)) {
					p->state.xdelta = -p->state.xdelta;
				}
				p->history_count = 0;
				p->charge_tick = CHARGE_TICKS_SECONDS * FPS;
			}
			if (p->charge_tick == 0 && bounced_x && best_dy != attract_radius) {
				if ((best_dy > 0 && p->state.ydelta < 0) || (best_dy < 0 && p->state.ydelta > 0)) {
					p->state.ydelta = -p->state.ydelta;
				}
				p->history_count = 0;
				p->charge_tick = CHARGE_TICKS_SECONDS * FPS;
			}
		}
		bool trapped = false;
		if (p->charge_tick == 0 && speed_mod == 1) {
			for (size_t i = 0; i < p->history_count; i++) {
				PenguinState *old = &p->history[i];
				if (old->x == p->state.x && 
					old->y == p->state.y && 
					old->xdelta == p->state.xdelta && 
					old->ydelta == p->state.ydelta && 
					old->speedslower == p->state.speedslower &&
					old->phase == p->state.phase) {
					trapped = true;
					break;
				}
			}
			if (trapped) {
				int nudge = (random() % 2) ? 1 : -1;
				grid_mark(p->state.x, p->state.y, BLOCKWIDTH * p->size, BLOCKHEIGHT * p->size, ' ');
				if (penguin_occupy_block(p->state.x, p->state.y + nudge, p->size)) {
					p->state.y += nudge;
				} else if (penguin_occupy_block(p->state.x + nudge, p->state.y, p->size)) {
					p->state.x += nudge;
				}
				grid_mark(p->state.x, p->state.y, BLOCKWIDTH * p->size, BLOCKHEIGHT * p->size, '*');
				p->history_count = 0;
			} else {
				if (p->history_count == MAX_STATE_HISTORY) {
					for (size_t i = 1; i < p->history_count; i++) p->history[i - 1] = p->history[i];
					p->history_count--;
				}
				p->history[p->history_count++] = p->state;
			}
		}
	}
	if (shield_touched) return PENGUIN_HIT_SHIELD;
	if (hitline1 && hitline2) return PENGUIN_HIT_BOTH;
	if (hitline1) return PENGUIN_HIT_LINE1;
	if (hitline2) return PENGUIN_HIT_LINE2;
	if (bounced_x || bounced_y) return PENGUIN_HIT_WALL;
	return PENGUIN_HIT_NONE;
}
