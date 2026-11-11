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

#include <stdlib.h>
#include "transition.h"
#include "themes.h"
#include "mesh.h"
#include "game.h"

static Mesh* transition_mesh;
static TransitionType transition_type;
static int movement_count;
static float rotation_speed;
static float depth_speed;

void transition_init(TransitionType type) {
	transition_type = TRANSITION_COUNT;
	transition_mesh = mesh_generate_grid(COLS, ROWS, -160.0f, 160.0f, 120.0f, -120.0f, 0.0f, 1.0f, 320.0f / 512.0f, 1.0f - 240.0f / 256.0f, 1.0f, C3D_AspectRatioBot);
	if (!transition_mesh) return;
	mesh_capture_begin(transition_mesh);
	game_draw_bottom(mesh_target(transition_mesh));
	mesh_capture_end();
	transition_type = type;
	movement_count = 0;
	rotation_speed = 0.0f;
	depth_speed = 0.0f;
	switch(transition_type) {
	case TRANSITION_SPLIT:
		int split_col = COLS / 2 - 1;
		for (int row = 0; row < ROWS; row++) {
			float split_x = -160.0f + (split_col + 0.5f) * (320.0f / COLS);

			for (int col = 0; col < COLS; col++) {
				for (int t = 0; t < 2; t++) {
					int i = (row * COLS + col) * 2 + t;
					float x = mesh_triangle_center_x(transition_mesh, i);
					mesh_shard(transition_mesh, i, x < split_x ? -3.0f : 3.0f, 0.0f, 1.0f);
				}
			}

			split_col += rand() % 3 - 1;
			if (split_col < COLS / 2 - 4) split_col = COLS / 2 - 4;
			if (split_col > COLS / 2 + 3) split_col = COLS / 2 + 3;
		}
		break;
	case TRANSITION_EXPLODE_OUT:
		for (int i = 0; i < mesh_vertex_count(transition_mesh) / 3; i++) {
			mesh_shard(transition_mesh, i, (rand() % 4 + 3) * (rand() % 2 ? 1 : -1), (rand() % 4 + 3) * (rand() % 2 ? 1 : -1), 1.0f);
		}
		break;
	case TRANSITION_EXPLODE_IN:
		for (int i = 0; i < mesh_vertex_count(transition_mesh) / 3; i++) {
			mesh_shard(transition_mesh, i, (rand() % 4 + 3) * (rand() % 2 ? 1 : -1), (rand() % 4 + 3) * (rand() % 2 ? 1 : -1), 1.0f);
		}
		mesh_offset_origin(transition_mesh, -70.0f);
		break;
	case TRANSITION_FLIP:
		rotation_speed = C3D_AngleFromDegrees(90.0f) / 70.0f;
		depth_speed = 50.0f / 70.0f;
		break;
	default:
		break;
	}
	mesh_move(transition_mesh, 0.0f);
}

void transition_draw(C3D_RenderTarget *target) {
	if (!transition_active()) return;
	switch(transition_type) {
	case TRANSITION_SPLIT:
		u32 alpha = 255 * (70 - movement_count) / 70;
		C2D_DrawRectSolid(0, 0, 0.47f, WIDTH, HEIGHT, (theme.color.background & 0x00FFFFFF) | (alpha << 24));
		break;
	default:
		break;
	}
	mesh_draw(transition_mesh, target);
}

bool transition_update(void) {
	if (!transition_active()) return false;
	if (movement_count < 70) {
		movement_count++;
		mesh_move(transition_mesh, (float)movement_count);
		mesh_rotate(transition_mesh, 0.0f, rotation_speed * movement_count, depth_speed * movement_count);
		return true;
	}
	return false;
}

bool transition_active(void) {
	return transition_mesh && transition_type < TRANSITION_COUNT;
}

bool transition_hides_screen(void) {
	return transition_active() && transition_type == TRANSITION_EXPLODE_IN;
}

void transition_close(void) {
	mesh_destroy(transition_mesh);
	transition_mesh = NULL;
	transition_type = TRANSITION_COUNT;
}
