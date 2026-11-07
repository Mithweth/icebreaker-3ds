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
#include <citro3d.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include "logo.h"
#include "mesh.h"

typedef enum {
	LOGO_NORMAL,
	LOGO_ROTATE,
	LOGO_EXPLODE,
} LogoState;

typedef struct {
	float xdelta;
	float ydelta;
	float angle_y;
	float angle_x;
	float position_z;
	LogoState status;
} Logo;

static Logo logo;

static Mesh *logo_mesh;
static size_t explosion_count;

static const char *logo_chars = "ICEBRAK";

static const u8 logo_glyphs[][7] = {
	{31,  4,  4,  4,  4,  4, 31}, // I
	{15, 16, 16, 16, 16, 16, 15}, // C
	{31, 16, 16, 30, 16, 16, 31}, // E
	{30, 17, 17, 30, 17, 17, 30}, // B
	{30, 17, 17, 30, 20, 18, 17}, // R
	{14, 17, 17, 31, 17, 17, 17}, // A
	{17, 18, 20, 24, 20, 18, 17}  // K
};

static void logo_drawtext(const char *text, float y, float scale) {
	const u32 gradient[8] = {
		C2D_Color32(245, 253, 255, 255),
		C2D_Color32(218, 245, 255, 255),
		C2D_Color32(179, 228, 250, 255),
		C2D_Color32(135, 207, 240, 255),
		C2D_Color32(100, 179, 223, 255),
		C2D_Color32(70, 150, 208, 255),
		C2D_Color32(48, 117, 183, 255),
		C2D_Color32(25, 75, 145, 255)
	};

	float logo_width = (strlen(text) * 6 - 1) * scale;
	float x = (400.0f - logo_width) / 2.0f;
	y -= 7.0f * (scale - 1.0f) / 2.0f;
	for (int i = 0; text[i]; i++) {
		const char *p = strchr(logo_chars, text[i]);
		if (!p) continue;

		int glyph = p - logo_chars;
		for (int row = 0; row < 7; row++) {
			for (int col = 0; col < 5;) {
				if (!(logo_glyphs[glyph][row] & (1 << (4 - col)))) {
					col++;
					continue;
				}

				int start = col;
				while (col < 5 && (logo_glyphs[glyph][row] & (1 << (4 - col)))) col++;
				C2D_DrawRectangle(x + (i * 6 + start) * scale, y + row * scale, 0.3f, (col - start) * scale, scale, gradient[row], gradient[row], gradient[row + 1], gradient[row + 1]);
				for (int c = start; c < col; c++) {
					if (row == 0 || !(logo_glyphs[glyph][row - 1] & (1 << (4 - c)))) {
						C2D_DrawRectSolid(x + (i * 6 + c) * scale, y + row * scale, 0.3f, scale, 1.5f, C2D_Color32(245, 252, 255, 230));
					}
				}
			}
		}
	}
}

void logo_rotate(float x, float y) {
	logo.xdelta = x;
	logo.ydelta = y;
	logo.status = LOGO_ROTATE;
}

void logo_explode(int xdelta, int ydelta) {
	for (int i = 0; i < mesh_vertex_count(logo_mesh) / 3; i++) {
		mesh_shard(logo_mesh, i, (rand() % xdelta - xdelta / 2) * 0.01f, -(rand() % ydelta + 2) * 0.01f, 1.0f);
	}
	explosion_count = 0;
	logo.status = LOGO_EXPLODE;
}

void logo_update(void) {
	if (!logo_mesh) return;
	switch(logo.status) {
	case LOGO_NORMAL:
		if (logo.angle_y > 0.0f) logo.angle_y -= 0.02f;
		if (logo.angle_x > 0.0f) logo.angle_x -= 0.02f;
		if (logo.position_z > 0.0f) logo.position_z -= 0.02f;
		mesh_rotate(logo_mesh, logo.angle_x, logo.angle_y, logo.position_z);
		break;
	case LOGO_ROTATE:
		if (logo.ydelta != 0.0f) {
			if (logo.angle_y < M_PI) logo.position_z += 0.1f;
			if (logo.angle_y > 3 * M_PI) logo.position_z -= 0.1f;
			if (logo.angle_y >= 4 * M_PI) {
				logo.angle_y = 0.0f;
				logo.ydelta = 0.0f;
			} else {
				logo.angle_y += logo.ydelta;
			}
		}
		if (logo.xdelta != 0.0f) {
			if (logo.angle_x < M_PI) logo.position_z += 0.1f;
			if (logo.angle_x > 3 * M_PI) logo.position_z -= 0.1f;
			if (logo.angle_x >= 4 * M_PI) {
				logo.angle_x = 0.0f;
				logo.xdelta = 0.0f;
			} else {
				logo.angle_x += logo.xdelta;
			}
		}
		if (logo.xdelta == 0.0f && logo.ydelta == 0.0f) {
			logo.status = LOGO_NORMAL;
		}
		mesh_rotate(logo_mesh, logo.angle_x, logo.angle_y, logo.position_z);
		break;
	case LOGO_EXPLODE:
		if (explosion_count++ < 70) {
			mesh_move(logo_mesh, (float)explosion_count);
		} else {
			mesh_move(logo_mesh, 0);
			logo.status = LOGO_NORMAL;
		}
		break;
	default:
		break;
	}
}

void logo_draw(C3D_RenderTarget *target, float parallax) {
	if (!logo_mesh) return;
	mesh_translate(logo_mesh, parallax / 120.0f, 0.0f);
	mesh_draw(logo_mesh, target);
}

void logo_init(void) {
	logo_mesh = mesh_generate_grid(25, 8, -2.0f, 2.0f, 1.0f, -1.0f, 0.0f, 1.0f, 1.0f, 0.0f, 1.0f, C3D_AspectRatioTop);
	if (!logo_mesh) return;
	mesh_capture_begin(logo_mesh);
	C2D_ViewTranslate(56.0f, 8.0f);
	logo_drawtext("ICE", 75, 6.0f);
	logo_drawtext("BREAKER", 130, 6.0f);
	mesh_capture_end();
}

void logo_close(void) {
	mesh_destroy(logo_mesh);
	logo_mesh = NULL;
}
