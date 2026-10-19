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
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "game.h"
#include "lang.h"

#ifdef DEBUG

// Redirects stdout/stderr to the 3dslink host (run with `3dslink -s`).

#include <malloc.h>
#include <unistd.h>

static u32 *soc_buffer;
static int debug_fd = -1;

static void debug_init(void) {
	soc_buffer = memalign(0x1000, 0x100000);
	if (!soc_buffer) {
		return;
	}
	Result rc = socInit(soc_buffer, 0x100000);
	if (R_SUCCEEDED(rc)) {
		debug_fd = link3dsStdio();
	}
}

static void debug_close(void) {
	fflush(stdout);
	fflush(stderr);
	close(STDOUT_FILENO);
	close(STDERR_FILENO);
	if (debug_fd >= 0) {
		close(debug_fd);
	}
	socExit();
	free(soc_buffer);
}

#else
static void debug_init(void) {}
static void debug_close(void) {}
#endif

int main(int argc, char **argv) {
	int ret = 0;
	gfxInitDefault();
	gfxSet3D(true);
	romfsInit();
	debug_init();
	srand(time(NULL));
	C3D_Init(C3D_DEFAULT_CMDBUF_SIZE);
	C2D_Init(C2D_DEFAULT_MAX_OBJECTS * 4);
	C2D_Prepare();
	C3D_RenderTarget *top_left = C2D_CreateScreenTarget(GFX_TOP, GFX_LEFT);
	C3D_RenderTarget *top_right = C2D_CreateScreenTarget(GFX_TOP, GFX_RIGHT);
	C3D_RenderTarget *bottom = C2D_CreateScreenTarget(GFX_BOTTOM, GFX_LEFT);
	if (!lang_init()) {
		printf("Cannot initialize languages\n");
		ret = 1;
	}
	if (!game_init()) {
		printf("Cannot initialize game\n");
		ret = 1;
	}
	while (ret == 0 && aptMainLoop()) {
		hidScanInput();
		u32 keys = hidKeysDown();
		float parallax = 4.0f * osGet3DSliderState();
		touchPosition touch;
		hidTouchRead(&touch);
		if (!game_update(keys, touch)) {
			ret = 1;
		}
		C3D_FrameBegin(C3D_FRAME_SYNCDRAW);
		game_draw_bottom(bottom);
		game_draw_top(top_left, parallax);
		if (parallax > 0.0f) {
			game_draw_top(top_right, -parallax);
		}
		C3D_FrameEnd(0);
	}
	game_close();
	lang_close();
	C2D_Fini();
	C3D_Fini();
	romfsExit();
	gfxExit();
	debug_close();
	return ret;
}
