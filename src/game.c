// game.c

#include <3ds.h>
#include <citro2d.h>
#include <stdbool.h>
#include <stddef.h>
#include "gfxmap.h"
#include "intro.h"
#include "themes.h"
#include "level.h"

typedef enum {
    GAME_INTRO,
    GAME_LEVEL,
    GAME_MENU,
    GAME_INTERMISSION,
    GAME_OVER
} GameMode;

static GameMode game_mode;
static ScoreSheet levelscore;
static size_t level;

bool game_init(void) {
	game_mode = GAME_INTRO;
	if (!gfxmap_init()) {
		return false;
	}
	themes_settheme("linux");
	intro_init();
	level = 0;
	return true;
}

static bool game_start(void) {
	intro_close();
	level_close();
	if (!level_init(++level, 0)) {
		return false;
	}
	game_mode = GAME_LEVEL;
	return true;
}

bool game_update(u32 keys, touchPosition touch) {
	if (keys & KEY_START) {
		return false;
	}
	switch(game_mode) {
	case GAME_INTRO:
		bool intro_running = intro_update(keys, touch);
		if (!intro_running) {
			return game_start();
		}
	case GAME_LEVEL:
		return level_update(keys, touch);
	default:
		break;
	}
	return true;
}

void game_draw(C3D_RenderTarget *top, C3D_RenderTarget *bottom) {
	C2D_TargetClear(top, C2D_Color32(0, 0, 0, 255));
	C2D_TargetClear(bottom, C2D_Color32(0, 0, 0, 255));
	switch(game_mode) {
	case GAME_INTRO:
		C2D_SceneBegin(bottom);
		intro_draw();
	case GAME_LEVEL:
		C2D_SceneBegin(bottom);
		level_draw();
	default:
		break;
	}
	return;
}

void game_close(void) {
	gfxmap_close();
}