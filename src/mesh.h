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

#pragma once

#include <citro3d.h>

typedef struct Mesh Mesh;

void mesh_init(void);
void mesh_close(void);

Mesh *mesh_generate_grid(int columns, int rows, float left, float right, float top, float bottom, float u0, float v0, float u1, float v1, float alpha, float aspect);
void mesh_destroy(Mesh *mesh);

void mesh_move_triangle(Mesh *mesh, float progress, int i);
void mesh_move(Mesh *mesh, float progress);
int mesh_vertex_count(const Mesh* mesh);
void mesh_shard(Mesh *mesh, int i, float xdelta, float ydelta, float alpha);
float mesh_shard_alpha(const Mesh *mesh, int i);
void mesh_shard_set_alpha(Mesh *mesh, int i, float alpha);
void mesh_rotate(Mesh* mesh, float x, float y, float z);
void mesh_translate(Mesh* mesh, float x, float y);
float mesh_triangle_center_x(const Mesh *mesh, int i);
void mesh_offset_origin(Mesh *mesh, float offset);
C3D_RenderTarget* mesh_target(const Mesh* mesh);
void mesh_draw(Mesh *mesh, C3D_RenderTarget* target);
void mesh_capture_begin(const Mesh *mesh);
void mesh_capture_end(void);
