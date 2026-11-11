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
#include <citro3d.h>
#include <citro2d.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "mesh.h"
#include "vshader_shbin.h"

#define MESH_FOV 80.0f

typedef struct {
	float x, y, z;
	float u, v;
	float alpha;
} MeshVertex;

typedef struct {
	float xdelta;
	float ydelta;
	float alpha;
} MeshShard;

struct Mesh {
	MeshVertex *vertices;
	MeshVertex *original;
	int vertex_count;
	float angle_x;
	float angle_y;
	float depth;
	float base_depth;
	float translation_x;
	float translation_y;
	C3D_Mtx projection;
	C3D_Tex texture;
	C3D_RenderTarget* target;
	MeshShard *shards;
};

static DVLB_s *dvlb;
static shaderProgram_s program;
static C3D_AttrInfo attrInfo;
static int uProjection;
static int uModelView;

void mesh_init(void) {
	dvlb = DVLB_ParseFile((u32 *)vshader_shbin, vshader_shbin_size);
	shaderProgramInit(&program);
	shaderProgramSetVsh(&program, &dvlb->DVLE[0]);

	uProjection = shaderInstanceGetUniformLocation(program.vertexShader, "projection");
	uModelView = shaderInstanceGetUniformLocation(program.vertexShader, "modelView");

	AttrInfo_Init(&attrInfo);
	AttrInfo_AddLoader(&attrInfo, 0, GPU_FLOAT, 3);
	AttrInfo_AddLoader(&attrInfo, 1, GPU_FLOAT, 2);
	AttrInfo_AddLoader(&attrInfo, 2, GPU_FLOAT, 1);
}

void mesh_close(void) {
	shaderProgramFree(&program);
	DVLB_Free(dvlb);
}

void mesh_capture_begin(const Mesh* mesh) {
	C3D_FrameBegin(C3D_FRAME_SYNCDRAW);
	C2D_Prepare();
	C2D_TargetClear(mesh->target, C2D_Color32(0, 0, 0, 0));
	C2D_SceneBegin(mesh->target);
}

void mesh_capture_end(void) {
	C2D_Flush();
	C2D_ViewReset();
	C3D_FrameEnd(0);
}

int mesh_vertex_count(const Mesh* mesh) {
	if (!mesh) return 0;
	return mesh->vertex_count;
}

void mesh_shard(Mesh* mesh, int i, float xdelta, float ydelta, float alpha) {
	if (!mesh || i < 0 || i >= mesh->vertex_count / 3) return;
	mesh->shards[i].ydelta = ydelta;
	mesh->shards[i].xdelta = xdelta;
	mesh->shards[i].alpha = alpha;
}

float mesh_shard_alpha(const Mesh *mesh, int i) {
	if (!mesh || i < 0 || i >= mesh->vertex_count / 3) return 0.0f;
	return mesh->shards[i].alpha;
}

void mesh_shard_set_alpha(Mesh *mesh, int i, float alpha) {
	if (!mesh || i < 0 || i >= mesh->vertex_count / 3) return;
	mesh->shards[i].alpha = alpha;
}

void mesh_rotate(Mesh* mesh, float x, float y, float z) {
	if (!mesh) return;
	mesh->angle_x = x;
	mesh->angle_y = y;
	mesh->depth = z;
}

void mesh_translate(Mesh* mesh, float x, float y) {
	if (!mesh) return;
	mesh->translation_x = x;
	mesh->translation_y = y;
}

C3D_RenderTarget* mesh_target(const Mesh* mesh) {
	if (!mesh) return NULL;
	return mesh->target;
}

float mesh_triangle_center_x(const Mesh *mesh, int i) {
	if (!mesh || i < 0 || i >= mesh->vertex_count / 3) return 0.0f;
	int n = i * 3;
	return (mesh->original[n].x + mesh->original[n + 1].x + mesh->original[n + 2].x) / 3.0f;
}

void mesh_offset_origin(Mesh *mesh, float offset) {
	if (!mesh) return;
	for (int i = 0; i < mesh->vertex_count / 3; i++) {
		for (int j = 0; j < 3; j++) {
			int n = i * 3 + j;
			mesh->original[n].x += mesh->shards[i].xdelta * offset;
			mesh->original[n].y += mesh->shards[i].ydelta * offset;
		}
	}
}

Mesh *mesh_generate_grid(int columns, int rows, float left, float right, float top, float bottom, float u0, float v0, float u1, float v1, float alpha, float aspect) {
	Mesh *mesh = calloc(1, sizeof(Mesh));
	if (!mesh) return NULL;
	if (!C3D_TexInitVRAM(&mesh->texture, 512, 256, GPU_RGBA8)) {
		mesh_destroy(mesh);
		return NULL;
	}

	mesh->target = C3D_RenderTargetCreateFromTex(&mesh->texture, GPU_TEXFACE_2D, 0, -1);
	if (!mesh->target) {
		mesh_destroy(mesh);
		return NULL;
	}

	C3D_TexSetFilter(&mesh->texture, GPU_LINEAR, GPU_LINEAR);
	C3D_TexSetWrap(&mesh->texture, GPU_CLAMP_TO_EDGE, GPU_CLAMP_TO_EDGE);
	mesh->vertex_count = columns * rows * 6;
	size_t size = mesh->vertex_count * sizeof(MeshVertex);

	mesh->vertices = linearAlloc(size);
	mesh->original = malloc(size);
	if (!mesh->vertices || !mesh->original) {
		mesh_destroy(mesh);
		return NULL;
	}

	mesh->shards = calloc(mesh->vertex_count / 3, sizeof(MeshShard));
	if (!mesh->shards) {
		mesh_destroy(mesh);
		return NULL;
	}
	for (int i = 0; i < mesh->vertex_count / 3; i++) {
		mesh->shards[i].alpha = alpha;
	}
	float center_x = (left + right) / 2.0f;
	float center_y = (top + bottom) / 2.0f;
	for (int y = 0; y < rows; y++) {
		for (int x = 0; x < columns; x++) {
			int i = (y * columns + x) * 6;

			float x0 = left + (right - left) * x / columns - center_x;
			float x1 = left + (right - left) * (x + 1) / columns - center_x;
			float y0 = top + (bottom - top) * y / rows - center_y;
			float y1 = top + (bottom - top) * (y + 1) / rows - center_y;

			float tx0 = u0 + (u1 - u0) * x / columns;
			float tx1 = u0 + (u1 - u0) * (x + 1) / columns;
			float ty0 = v0 + (v1 - v0) * y / rows;
			float ty1 = v0 + (v1 - v0) * (y + 1) / rows;

			mesh->vertices[i]     = (MeshVertex){x0, y0, 0.0f, tx0, ty0, alpha};
			mesh->vertices[i + 1] = (MeshVertex){x1, y0, 0.0f, tx1, ty0, alpha};
			mesh->vertices[i + 2] = (MeshVertex){x1, y1, 0.0f, tx1, ty1, alpha};
			mesh->vertices[i + 3] = (MeshVertex){x1, y1, 0.0f, tx1, ty1, alpha};
			mesh->vertices[i + 4] = (MeshVertex){x0, y1, 0.0f, tx0, ty1, alpha};
			mesh->vertices[i + 5] = (MeshVertex){x0, y0, 0.0f, tx0, ty0, alpha};
		}
	}

	memcpy(mesh->original, mesh->vertices, size);
	float height = fabsf(bottom - top);
	mesh->base_depth = (height / 2.0f) / tanf(C3D_AngleFromDegrees(MESH_FOV) / 2.0f);
	Mtx_PerspTilt(&mesh->projection, C3D_AngleFromDegrees(MESH_FOV), aspect, 0.1f, 1000.0f, false);
	return mesh;
}

void mesh_destroy(Mesh *mesh) {
	if (!mesh) return;
	if (mesh->target) C3D_RenderTargetDelete(mesh->target);
	C3D_TexDelete(&mesh->texture);
	linearFree(mesh->vertices);
	free(mesh->original);
	free(mesh->shards);
	free(mesh);
}

void mesh_move_triangle(Mesh *mesh, float progress, int i) {
	if (!mesh || i < 0 || i >= mesh->vertex_count / 3) return;
	for (int j = 0; j < 3; j++) {
		int n = i * 3 + j;
		mesh->vertices[n].x = mesh->original[n].x + mesh->shards[i].xdelta * progress;
		mesh->vertices[n].y = mesh->original[n].y + mesh->shards[i].ydelta * progress;
		mesh->vertices[n].alpha = mesh->shards[i].alpha;
	}
}

void mesh_move(Mesh *mesh, float progress) {
	if (!mesh) return;
	for (int i = 0; i < mesh->vertex_count / 3; i++) {
		mesh_move_triangle(mesh, progress, i);
	}
}

void mesh_draw(Mesh *mesh, C3D_RenderTarget* target) {
	C2D_Flush();
	C3D_FrameDrawOn(target);
	GSPGPU_FlushDataCache(mesh->vertices, mesh->vertex_count * sizeof(MeshVertex));
	C3D_Mtx modelView;
	Mtx_Identity(&modelView);
	Mtx_RotateX(&modelView, mesh->angle_x, true);
	Mtx_RotateY(&modelView, mesh->angle_y, true);
	Mtx_Translate(&modelView, mesh->translation_x, mesh->translation_y, -mesh->base_depth - mesh->depth, false);

	C3D_BufInfo bufInfo;
	BufInfo_Init(&bufInfo);
	BufInfo_Add(&bufInfo, mesh->vertices, sizeof(MeshVertex), 3, 0x210);

	C3D_BindProgram(&program);
	C3D_SetAttrInfo(&attrInfo);
	C3D_SetBufInfo(&bufInfo);
	C3D_TexBind(0, &mesh->texture);

	C3D_TexEnv *env = C3D_GetTexEnv(0);
	C3D_TexEnvInit(env);
	C3D_TexEnvSrc(env, C3D_RGB, GPU_TEXTURE0, 0, 0);
	C3D_TexEnvFunc(env, C3D_RGB, GPU_REPLACE);
	C3D_TexEnvSrc(env, C3D_Alpha, GPU_TEXTURE0, GPU_PRIMARY_COLOR, 0);
	C3D_TexEnvFunc(env, C3D_Alpha, GPU_MODULATE);

	C3D_AlphaBlend(GPU_BLEND_ADD, GPU_BLEND_ADD, GPU_SRC_ALPHA, GPU_ONE_MINUS_SRC_ALPHA, GPU_SRC_ALPHA, GPU_ONE_MINUS_SRC_ALPHA);
	C3D_DepthTest(false, GPU_ALWAYS, GPU_WRITE_ALL);
	C3D_CullFace(GPU_CULL_NONE);

	C3D_FVUnifMtx4x4(GPU_VERTEX_SHADER, uProjection, &mesh->projection);
	C3D_FVUnifMtx4x4(GPU_VERTEX_SHADER, uModelView, &modelView);

	C3D_DrawArrays(GPU_TRIANGLES, 0, mesh->vertex_count);
	C2D_Prepare();
	C2D_SceneBegin(target);
}