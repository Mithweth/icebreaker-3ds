// game.c

#include <3ds.h>
#include <citro2d.h>
#include <stdbool.h>
#include <stddef.h>
#include "gfxmap.h"
#include "intro.h"
#include "themes.h"
#include "level.h"
#include "menu.h"
#include "intermission.h"

typedef enum {
    GAME_INTRO,
    GAME_LEVEL,
    GAME_MENU,
    GAME_INTERMISSION,
    GAME_OVER
} GameMode;

static GameMode game_mode;
static GameStats stats;

bool game_init(void) {
	game_mode = GAME_INTRO;
	if (!gfxmap_init()) {
		return false;
	}
	themes_settheme("linux");
	intro_init();
	menu_init();
	return true;
}

static void game_start(void) {
	intro_close();
	level_close();
	stats.score = 0;
	stats.level = 0;
	stats.dead = false;
	level_init(&stats);
	game_mode = GAME_LEVEL;
}

static void game_continue(void) {
	intro_close();
	level_close();
	stats.level++;
	stats.score += stats.basescore + stats.clearbonus + stats.lifebonus;
	level_init(&stats);
	game_mode = GAME_LEVEL;
}

static void game_intermission_start(void) {
	intermission_init(&stats);
	game_mode = GAME_INTERMISSION;
}

bool game_update(u32 keys, touchPosition touch) {
	if (keys & KEY_START) {
		return false;
	}
	switch(game_mode) {
	case GAME_INTRO:
		if (!intro_update(keys, touch)) {
			game_start();
		}
		break;
	case GAME_LEVEL:
		if (!level_update(keys, touch, &stats)) {
			game_intermission_start();
		}
		break;
	case GAME_INTERMISSION:
		if (!intermission_update(keys)) {
			if (stats.dead) {
				game_start();
			} else {
				game_continue();
			}
		}
		break;
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
		break;
	case GAME_LEVEL:
		C2D_SceneBegin(bottom);
		level_draw();
		break;
	case GAME_INTERMISSION:
		C2D_SceneBegin(bottom);
		level_draw();
		intermission_draw();
		break;
	default:
		break;
	}
	C2D_SceneBegin(top);
	menu_draw(&stats);
	return;
}

void game_close(void) {
	gfxmap_close();
	menu_close();
}