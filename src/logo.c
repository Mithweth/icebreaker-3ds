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

#include "logo.h"
#include "vshader_shbin.h"

static C3D_Tex logo_tex;
static C3D_RenderTarget* logo_target;
static DVLB_s* dvlb;
static shaderProgram_s program;
static int uProjection;
static int uModelView;
static C3D_AttrInfo logo_attr_info;
static C3D_BufInfo logo_buf_info;
static C3D_Mtx projection;
static C3D_Mtx modelView;

#define LOGO_VERTEX_COUNT (25 * 8 * 6)

typedef enum {
	LOGO_NORMAL,
	LOGO_ROTATE,
	LOGO_EXPLODE,
} LogoState;

typedef struct {
	float xdelta;
	float ydelta;
	float zdelta;
	float angle_y;
	float angle_x;
	float position_z;
	LogoState status;
} Logo;

static Logo logo;

typedef struct {
	float x, y, z;
	float u, v;
} LogoVertex;

typedef struct {
    float xdelta;
    float ydelta;
    float angle;
} LogoShard;

static LogoShard shards[LOGO_VERTEX_COUNT / 3];
static LogoVertex logoVertices[LOGO_VERTEX_COUNT];
static LogoVertex* vertices;

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
	logo.zdelta = 0.1f;
	logo.status = LOGO_ROTATE;
}

void logo_explode(int xdelta, int ydelta) {
    for (int i = 0; i < LOGO_VERTEX_COUNT / 3; i++) {
    	shards[i].ydelta = -(rand() % ydelta + 2) * 0.01f;
        shards[i].xdelta = (rand() % xdelta - xdelta / 2) * 0.01f;
    }
        //shards[i].vy -= 0.001f;
    logo.status = LOGO_EXPLODE;
}

void logo_update(void) {
	switch(logo.status) {
	case LOGO_NORMAL:
		if (logo.angle_y > 0.0f) logo.angle_y -= 0.02f;
		if (logo.angle_x > 0.0f) logo.angle_x -= 0.02f;
		if (logo.position_z > 1.0f) logo.position_z -= 0.02f;
		break;
	case LOGO_ROTATE:
		if (logo.ydelta != 0.0f) {
			if (logo.angle_y < M_PI) logo.position_z += logo.zdelta;
			if (logo.angle_y > 3 * M_PI) logo.position_z -= logo.zdelta;
			if (logo.angle_y >= 4 * M_PI) {
				logo.angle_y = 0.0f;
				logo.ydelta = 0.0f;
			} else {
				logo.angle_y += logo.ydelta;
			}
		}
		if (logo.xdelta != 0.0f) {
			if (logo.angle_x < M_PI) logo.position_z += logo.zdelta;
			if (logo.angle_x > 3 * M_PI) logo.position_z -= logo.zdelta;
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
		break;
	case LOGO_EXPLODE:
		bool done = true;
	    for (int i = 0; i < LOGO_VERTEX_COUNT / 3; i++) {
	        for (int j = 0; j < 3; j++) {
	            vertices[i * 3 + j].y += shards[i].ydelta;
	            vertices[i * 3 + j].x += shards[i].xdelta;
	            if (vertices[i * 3 + j].y > -2.0f) done = false;
	        }
	    }
	    if (done) {
	    	memcpy(vertices, logoVertices, sizeof(logoVertices));
	    	logo.status = LOGO_NORMAL;
	    }
	    GSPGPU_FlushDataCache(vertices, LOGO_VERTEX_COUNT * sizeof(LogoVertex));
	    break;
	default:
		break;
	}
}

void logo_draw(C3D_RenderTarget *target) {
	C2D_Flush();
	C3D_FrameDrawOn(target);
	Mtx_Identity(&modelView);
	Mtx_RotateX(&modelView, logo.angle_x, true);
	Mtx_RotateY(&modelView, logo.angle_y, true);
	Mtx_Translate(&modelView, 0.0f, 0.0f, -logo.position_z, false);
	C3D_BindProgram(&program);
	C3D_SetAttrInfo(&logo_attr_info);
	C3D_SetBufInfo(&logo_buf_info);
	C3D_TexBind(0, &logo_tex);

	C3D_TexEnv* env = C3D_GetTexEnv(0);
	C3D_TexEnvInit(env);
	C3D_TexEnvSrc(env, C3D_Both, GPU_TEXTURE0, 0, 0);
	C3D_TexEnvFunc(env, C3D_Both, GPU_REPLACE);

	C3D_AlphaBlend(GPU_BLEND_ADD, GPU_BLEND_ADD, GPU_SRC_ALPHA, GPU_ONE_MINUS_SRC_ALPHA, GPU_SRC_ALPHA, GPU_ONE_MINUS_SRC_ALPHA);
	C3D_DepthTest(false, GPU_ALWAYS, GPU_WRITE_ALL);
	C3D_CullFace(GPU_CULL_NONE);

	C3D_FVUnifMtx4x4(GPU_VERTEX_SHADER, uProjection, &projection);
	C3D_FVUnifMtx4x4(GPU_VERTEX_SHADER, uModelView, &modelView);

	C3D_DrawArrays(GPU_TRIANGLES, 0, LOGO_VERTEX_COUNT);
	C2D_Prepare();
	C2D_SceneBegin(target);
}

void logo_init(void) {
	logo.angle_x = 0.0f;
	logo.angle_y = 0.0f;
	logo.position_z = 1.0f;
	logo.status = LOGO_NORMAL;
	C3D_TexInitVRAM(&logo_tex, 512, 256, GPU_RGBA8);
	for(int y = 0; y < 8; y++) {
		for(int x = 0; x < 25; x++) {
			int index = (y * 25 + x) * 6;
	        float x0 = -2.0f + 4.0f * x / 25.0f;
	        float x1 = -2.0f + 4.0f * (x + 1) / 25.0f;
	        float y0 = 1.0f - 2.0f * y / 8.0f;
	        float y1 = 1.0f - 2.0f * (y + 1) / 8.0f;
	        float u0 = x / 25.0f;
	        float u1 = (x + 1) / 25.0f;
	        float v0 = 1 - y / 8.0f;
	        float v1 = 1 - (y + 1) / 8.0f;
	        logoVertices[index] = (LogoVertex){x0, y0, 0.0f, u0, v0};
	        logoVertices[index + 1] = (LogoVertex){x1, y0, 0.0f, u1, v0};
	        logoVertices[index + 2] = (LogoVertex){x1, y1, 0.0f, u1, v1};
	        logoVertices[index + 3] = (LogoVertex){x1, y1, 0.0f, u1, v1};
	        logoVertices[index + 4] = (LogoVertex){x0, y1, 0.0f, u0, v1};
	        logoVertices[index + 5] = (LogoVertex){x0, y0, 0.0f, u0, v0};
	    }
	}

	vertices = linearAlloc(sizeof(logoVertices));
	memcpy(vertices, logoVertices, sizeof(logoVertices));
	AttrInfo_Init(&logo_attr_info);
	AttrInfo_AddLoader(&logo_attr_info, 0, GPU_FLOAT, 3);
	AttrInfo_AddLoader(&logo_attr_info, 1, GPU_FLOAT, 2);
	BufInfo_Init(&logo_buf_info);
	BufInfo_Add(&logo_buf_info, vertices, sizeof(LogoVertex), 2, 0x10);
	dvlb = DVLB_ParseFile((u32*)vshader_shbin, vshader_shbin_size);
	shaderProgramInit(&program);
	shaderProgramSetVsh(&program, &dvlb->DVLE[0]);
	uProjection = shaderInstanceGetUniformLocation(program.vertexShader, "projection");
	uModelView = shaderInstanceGetUniformLocation(program.vertexShader, "modelView");
	logo_target = C3D_RenderTargetCreateFromTex(&logo_tex, GPU_TEXFACE_2D, 0, -1);

	C3D_TexSetFilter(&logo_tex, GPU_LINEAR, GPU_LINEAR);
	C3D_TexSetWrap(&logo_tex, GPU_CLAMP_TO_EDGE, GPU_CLAMP_TO_EDGE);

	C3D_FrameBegin(C3D_FRAME_SYNCDRAW);

	C2D_Prepare();
	C2D_TargetClear(logo_target, C2D_Color32(0, 0, 0, 0));
	C2D_SceneBegin(logo_target);
	C2D_ViewTranslate(56.0f, 8.0f);

	logo_drawtext("ICE", 75, 6.0f);
	logo_drawtext("BREAKER", 130, 6.0f);

	C2D_Flush();
	C2D_ViewReset();

	Mtx_PerspTilt(&projection, C3D_AngleFromDegrees(80.0f), C3D_AspectRatioTop, 0.1f, 100.0f, false);

	C3D_FrameEnd(0);
}

void logo_close(void) {
	linearFree(vertices);
	C3D_RenderTargetDelete(logo_target);
	C3D_TexDelete(&logo_tex);
}
