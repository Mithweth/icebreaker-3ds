// main.c
#include <citro2d.h>
#include <stdlib.h>
#include <3ds.h>
#include <time.h>
#include "game.h"

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

static aptHookCookie apt_cookie;

static void apt_callback(APT_HookType hook, void *param) {
	if (hook == APTHOOK_ONRESTORE || hook == APTHOOK_ONWAKEUP) {
		// on restore
	}
}

int main(int argc, char **argv) {
	int ret = 0;
	gfxInitDefault();
	romfsInit();
	debug_init();
	srand(time(NULL));
	C3D_Init(C3D_DEFAULT_CMDBUF_SIZE);
	C2D_Init(C2D_DEFAULT_MAX_OBJECTS);
	C2D_Prepare();
	C3D_RenderTarget *top = C2D_CreateScreenTarget(GFX_TOP, GFX_LEFT);
	C3D_RenderTarget *bottom = C2D_CreateScreenTarget(GFX_BOTTOM, GFX_LEFT);
	if (!game_init()) {
		printf("Cannot initialize game\n");
		ret = 1;
	}
	aptHook(&apt_cookie, apt_callback, NULL);
	while (ret == 0 && aptMainLoop()) {
		hidScanInput();
		u32 keys = hidKeysDown();
		touchPosition touch;
		hidTouchRead(&touch);
		if (keys & KEY_START) {
			break;
		}
		if (!game_update(keys, touch)) {
			ret = 1;
		}
		C3D_FrameBegin(C3D_FRAME_SYNCDRAW);
		game_draw(top, bottom);
		C3D_FrameEnd(0);
	}

	aptUnhook(&apt_cookie);
	game_close();
	C2D_Fini();
	C3D_Fini();
	romfsExit();
	gfxExit();
	debug_close();
	return ret;
}
