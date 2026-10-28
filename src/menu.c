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

#include "menu.h"

#include <3ds.h>
#include <citro2d.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include "themes.h"
#include "game.h"
#include "text.h"
#include "options.h"
#include "grid.h"
#include "grid_maps.h"
#include "hiscore.h"
#include "lang.h"
#include "flying_sprite.h"
#include "snowflake.h"

#define GRID_PREVIEW_MARGIN		20

typedef enum {
	CONFIRM_DIFFICULTY,
	CONFIRM_GRID,
	CONFIRM_MODE,
} DialogConfirmType;

typedef enum {
	DIALOG_NORMAL,
	DIALOG_CONFIRM,
	DIALOG_HIGHSCORES,
	DIALOG_HELP,
} DialogType;

typedef enum {
	MENU_NEW_GAME,
	MENU_OPTIONS,
	MENU_LANGUAGE,
	MENU_DIFFICULTY,
	MENU_THEME,
	MENU_GRID,
	MENU_MODE,
	MENU_EDITOR,
	MENU_BACK,
	MENU_HIGHSCORES,
	MENU_HELP,
} MenuChoice;

typedef struct {
	const char *label;
	MenuChoice choice;
} MenuEntry;

static DialogType dialog_type;
static bool yes_selected;
static const char *logo_chars = "ICEBRAK";
static size_t selected;
static bool options_selected;
static float logo_scale;
static float logo_parallax;
static GameMode selected_game_mode;
static DialogConfirmType dialog_confirm_type;
static size_t help_page;

static MenuEntry main_menu[] = {
	{"MENU_NEW_GAME", MENU_NEW_GAME},
	{"MENU_OPTIONS", MENU_OPTIONS},
	{"MENU_HIGH_SCORES", MENU_HIGHSCORES},
	{"MENU_EDITOR", MENU_EDITOR},
	{"MENU_HELP", MENU_HELP},
};
static MenuEntry options_menu[] = {
	{"MENU_LANGUAGE", MENU_LANGUAGE},
	{"MENU_DIFFICULTY", MENU_DIFFICULTY},
	{"MENU_MODE", MENU_MODE},
	{"MENU_GRID", MENU_GRID},
	{"MENU_THEME", MENU_THEME},
	{"MENU_BACK", MENU_BACK},
};
static const u8 logo_glyphs[][7] = {
	{31,  4,  4,  4,  4,  4, 31}, // I
	{15, 16, 16, 16, 16, 16, 15}, // C
	{31, 16, 16, 30, 16, 16, 31}, // E
	{30, 17, 17, 30, 17, 17, 30}, // B
	{30, 17, 17, 30, 20, 18, 17}, // R
	{14, 17, 17, 31, 17, 17, 17}, // A
	{17, 18, 20, 24, 20, 18, 17}  // K
};

static void logo_drawtext(const char *text, float y, float parallax, float scale) {
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
	float x = (400.0f - logo_width) / 2.0f + parallax / 2;
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

static void menu_draw_rounded_rect(float x, float y, float w, float h, u32 color) {
	if (w <= 0 || h <= 0) return;
	float r = fminf(w / 2.0f, h / 2.0f);
	C2D_DrawRectSolid(x + r, y + h / 2.0f - r, 0.6f, w - 2.0f * r, 2.0f * r, color);
	C2D_DrawCircleSolid(x + r, y + h / 2.0f, 0.6f, r, color);
	C2D_DrawCircleSolid(x + w - r, y + h / 2.0f, 0.6f, r, color);
}

static void progress_bar_draw(int pct) {
	if (pct < 0) pct = 0;
	if (pct > 100) pct = 100;
	menu_draw_rounded_rect(10.0f, 220.0f, 380.0f, 14.0f, theme.color.progressbaroutline);
	menu_draw_rounded_rect(12.0f, 222.0f, 376.0f, 10.0f, theme.color.progressbarinner);
	menu_draw_rounded_rect(12.0f, 222.0f, (376.0f) * pct / 100.0f, 10.0f, theme.color.progressbarfill);
}


void menu_bottom_draw(bool pause) {
	if (pause) {
		const MenuEntry *choices = options_selected ? options_menu : main_menu;
		if (choices[selected].choice == MENU_GRID && !grid_maps_random(grid_maps_selected_name())) {
			C2D_DrawRectSolid(0, 0, 0.86f, WIDTH, HEIGHT, theme.color.background & 0x90FFFFFF);
			int w = (WIDTH - GRID_PREVIEW_MARGIN * 2) / GRID_MAPS_COLS;
			int h = (HEIGHT - GRID_PREVIEW_MARGIN * 2) / GRID_MAPS_ROWS;
			int cell = (w < h) ? w : h;
			int grid_w = cell * GRID_MAPS_COLS;
			int grid_h = cell * GRID_MAPS_ROWS;
			int x = (WIDTH  - grid_w) / 2;
			int y = (HEIGHT - grid_h) / 2;
			C2D_DrawRectSolid(x - 2, y - 2, 0.87f, grid_w + 4, grid_h + 4, theme.color.gridline);
			C2D_DrawRectSolid(x, y, 0.88f, grid_w, grid_h, theme.color.background);
			for(int j = 0; j < GRID_MAPS_ROWS; j++) {
				for(int i = 0; i < GRID_MAPS_COLS; i++) {
					if (grid_maps_selected_at(i, j) == '.') continue;
					C2D_DrawRectSolid(x + i * cell, y + j * cell, 0.89f, cell - 1, cell - 1, theme.color.gridhighlight);
				}
			}
		}
		C2D_DrawRectSolid(0, 0, 0.9f, WIDTH, HEIGHT, C2D_Color32(0, 0, 0, 192));
		text_put(100, 110, 1.0f, 4, theme.color.normaltext, lang_get("MENU_PAUSE"));
	}
}

void menu_draw(bool pause, const GameStats *stats, float parallax) {
	C2D_DrawRectSolid(0, 0, 0.0f, 400.0f, 240.0f, theme.color.background);
	char tmptext[30]; // should be plenty big.
	text_put(175 + parallax, 10, 0.3f, 2, theme.color.normaltext, lang_get("MENU_SCORE"));
	text_put(10 + parallax, 10, 0.3f, 2, theme.color.normaltext, lang_get("MENU_LEVEL"));
	text_rput(385 + parallax, 10, 0.3f, 2, theme.color.normaltext, lang_get("MENU_HIGH_SCORE"));
	snprintf(tmptext,30,"%d %%",stats->clear);
	text_rput(390 + parallax, 200, 0.3f, 2, theme.color.normaltext, tmptext);
	snprintf(tmptext,30,"%ld",stats->score + stats->basescore);
	text_cput(200 + parallax, 25, 0.3f, 2, theme.color.normaltext, tmptext);
	snprintf(tmptext,30,"%d",stats->level);
	text_put(20 + parallax, 25, 0.3f, 2, theme.color.normaltext, tmptext);

	C2D_DrawImageAt(theme.spriteimage[0], 10.0f + parallax, 205.0f - theme.spriteimage[0].subtex->height / 2, 0.1f, NULL, 1.0f, 1.0f);
	snprintf(tmptext,30,"%d",stats->lives);
	text_put(30.0f + parallax, 200.0f, 0.1f, 2, theme.color.normaltext, tmptext);
	if (options_modern_mode()) {
		C2D_DrawImageAt(theme.energybitmap, 10.0f + parallax, 180.0f, 0.1f, NULL, 1.0f, 1.0f);
		snprintf(tmptext, 30, "%zu", stats->energy);
		text_put(30.0f + parallax, 184.0f, 0.1f, 2, theme.color.normaltext, tmptext);
	}
	if (!grid_maps_random(grid_maps_current_name())) {
		snprintf(tmptext, 30,"%ld", hiscore_get(options_game_mode(), grid_maps_current_name(), 0)->score);
		text_rput(385 + parallax, 25, 0.3f, 2, theme.color.normaltext, tmptext);
	}
	progress_bar_draw(stats->clear);
	snowflake_draw(parallax);
	if (!pause) {
		if (flying_sprite_celebrating()) {
			logo_scale += 0.04f;
			logo_parallax += (parallax > 0.0f) ? 0.01f : -0.01f;
		} else {
			if (logo_scale > 6.0f) {
				logo_scale -= 0.04f;
				logo_parallax -= (parallax > 0.0f) ? 0.01f : -0.01f;
			}
		}
		logo_drawtext("ICE", 75, logo_parallax + parallax, logo_scale);
		logo_drawtext("BREAKER", 130, logo_parallax + parallax, logo_scale);
		flying_sprite_draw(parallax);
		return;
	}
	switch(dialog_type) {
	case DIALOG_NORMAL:
		const MenuEntry *choices = options_selected ? options_menu : main_menu;
		size_t count = options_selected ? sizeof(options_menu) / sizeof(options_menu[0]) : sizeof(main_menu) / sizeof(main_menu[0]);
		C2D_DrawRectSolid(50.0f, 50.0f, 0.3f, 300.0f, 25.0f * count + 5.0f, theme.color.menuhighlight);
		C2D_DrawRectSolid(52.0f, 52.0f, 0.31f, 296.0f, 25.0f * count + 1.0f, theme.color.background);
	
		for(size_t i = 0; i < count; i++) {
			u32 option_color = theme.color.normaltext;
			if (selected == i) {
				C2D_DrawRectSolid(50.0f, 56 + 24 * i, 0.35f, 300.0f, 23.0f, theme.color.menuhighlight);
				option_color = theme.color.background;
			}
			text_put(60, 61 + 24 * i, 0.4f, 3, option_color, lang_get(choices[i].label));
			
			switch(choices[i].choice) {
				case MENU_LANGUAGE:
					text_rput(340, 61 + 24 * i, 0.4f, 3, option_color, lang_selected_name());
					break;
				case MENU_DIFFICULTY:
					const char* difficulty;
					switch(options_difficulty_selected()) {
					case EASY:
						difficulty = lang_get("DIFFICULTY_EASY");
						break;
					case HARD:
						difficulty = lang_get("DIFFICULTY_HARD");
						break;
					default:
						difficulty = lang_get("DIFFICULTY_NORMAL");
						break;
					}
					text_rput(340, 61 + 24 * i, 0.4f, 3, option_color, difficulty);
					break;
				case MENU_MODE:
					text_rput(340, 61 + 24 * i, 0.4f, 3, option_color, (selected_game_mode == GAME_MODE_MODERN) ? lang_get("MODE_MODERN") : lang_get("MODE_ORIGINAL"));
					break;
				case MENU_GRID:
					text_rput(340, 61 + 24 * i, 0.4f, 3, option_color, grid_maps_selected_name());
					break;
				case MENU_THEME:
					text_rput(340, 61 + 24 * i, 0.4f, 3, option_color, themes_current_name());
					break;
				default:
					break;
			}
		}
		break;
	case DIALOG_CONFIRM:
		C2D_DrawRectSolid(50.0f, 40.0f, 0.3f, 300.0f, 155.0f, theme.color.menuhighlight);
		C2D_DrawRectSolid(52.0f, 42.0f, 0.31f, 296.0f, 151.0f, theme.color.background);
		for(int i = 0; i < 3; i++) {
			char confirm_key[24];
			switch(dialog_confirm_type) {
			case CONFIRM_DIFFICULTY:
				snprintf(confirm_key, 24, "QUESTION_DIFFICULTY_%d", i + 1);
				break;
			case CONFIRM_GRID:
				snprintf(confirm_key, 24, "QUESTION_GRID_%d", i + 1);
				break;
			case CONFIRM_MODE:
				snprintf(confirm_key, 24, "QUESTION_MODE_%d", i + 1);
				break;
			default:
				break;
			}
			text_put(60, 60 + 22 * i, 0.4f, 3, theme.color.normaltext, lang_get(confirm_key));
		}

		int yes_w = 20.0f + text_getwidth(3, lang_get("CHOICE_YES"));
		int no_w = 20.0f + text_getwidth(3, lang_get("CHOICE_NO"));
		int box_w = (yes_w > no_w) ? yes_w : no_w;
		if (yes_selected) {
			C2D_DrawRectSolid(110.0f - box_w / 2, 145, 0.35f, box_w, 23.0f, theme.color.menuhighlight);
			text_cput(110.0f, 150, 0.4f, 3, theme.color.background, lang_get("CHOICE_YES"));
			text_cput(280.0f, 150, 0.4f, 3, theme.color.normaltext, lang_get("CHOICE_NO"));
		} else {
			C2D_DrawRectSolid(280.0f - box_w / 2, 145, 0.35f, box_w, 23.0f, theme.color.menuhighlight);
			text_cput(110.0f, 150, 0.4f, 3, theme.color.normaltext, lang_get("CHOICE_YES"));
			text_cput(280.0f, 150, 0.4f, 3, theme.color.background, lang_get("CHOICE_NO"));
		}
		break;
	case DIALOG_HIGHSCORES:
		C2D_DrawRectSolid(50.0f, 40.0f, 0.3f, 300.0f, 155.0f, theme.color.menuhighlight);
		C2D_DrawRectSolid(52.0f, 42.0f, 0.31f, 296.0f, 151.0f, theme.color.background);
		text_cput(200, 50, 0.4f, 3, theme.color.bonusscrolltext, lang_get("MENU_HIGH_SCORES"));
		text_cput(200, 70, 0.4f, 3, theme.color.normaltext, grid_maps_selected_name());
		for(int i = 0; i < HISCORENUM; i++) {
			int y = 95 + 20 * i;
			HighScore* user = hiscore_get(options_game_mode(), grid_maps_selected_name(), i);
			char nb[4];
			snprintf(nb, 4, "%d.", i + 1);
			char score[10];
			snprintf(score, 10, "%ld", user->score);
			text_put(60, y, 0.4f, 2, theme.color.normaltext, nb);
			text_put(100, y, 0.4f, 2, theme.color.normaltext, user->name);
			text_rput(340, y, 0.4f, 2, theme.color.normaltext, score);
		}
		break;
	case DIALOG_HELP:
		C2D_DrawRectSolid(50.0f, 40.0f, 0.3f, 300.0f, 155.0f, theme.color.menuhighlight);
		C2D_DrawRectSolid(52.0f, 42.0f, 0.31f, 296.0f, 151.0f, theme.color.background);
		for(int i = 0; i < 13; i++) {
			char helpkey[24];
			snprintf(helpkey, 24, "MENU_HELP_DETAIL_%d", 13 * help_page + i + 1);
			text_put(60, 48 + 11 * i, 0.4f, 1, theme.color.normaltext, lang_get(helpkey));
		}
		char page_number[6];
		snprintf(page_number, sizeof(page_number), "%d / 3", help_page + 1);
		text_rput(340.0f, 48, 0.4f, 1, theme.color.normaltext, page_number);
		break;
	default:
		break;
	}
}

bool menu_update(bool pause, u32 keys) {
	flying_sprite_update();
	snowflake_update();
	if (!pause) {
		return false;
	}
	switch(dialog_type) {
	case DIALOG_NORMAL:
		const MenuEntry *choices = options_selected ? options_menu : main_menu;
		size_t count = options_selected ? sizeof(options_menu) / sizeof(options_menu[0]) : sizeof(main_menu) / sizeof(main_menu[0]);
		if ((keys & KEY_UP) && (selected > 0)) {
			selected--;
		}
		if ((keys & KEY_DOWN) && (selected < count - 1)) {
			selected++;
		}
		if (keys & KEY_LEFT) {
			switch(choices[selected].choice) {
			case MENU_LANGUAGE:
				lang_prev();
				break;
			case MENU_DIFFICULTY:
				options_difficulty_prev();
				break;
			case MENU_THEME:
				themes_prev();
				grid_generate_colors();
				options_write();
				break;
			case MENU_GRID:
				grid_maps_prev();
				break;
			case MENU_MODE:
				selected_game_mode = (selected_game_mode == GAME_MODE_ORIGINAL) ? GAME_MODE_MODERN : GAME_MODE_ORIGINAL;
				break;
			default:
				break;
			}
		}
		if (keys & KEY_RIGHT) {
			switch(choices[selected].choice) {
			case MENU_LANGUAGE:
				lang_next();
				break;
			case MENU_DIFFICULTY:
				options_difficulty_next();
				break;
			case MENU_THEME:
				themes_next();
				grid_generate_colors();
				options_write();
				break;
			case MENU_GRID:
				grid_maps_next();
				break;
			case MENU_MODE:
				selected_game_mode = (selected_game_mode == GAME_MODE_ORIGINAL) ? GAME_MODE_MODERN : GAME_MODE_ORIGINAL;
				break;
			default:
				break;
			}
		}
		if ((keys & KEY_B) && options_selected) {
			options_selected = false;
			options_difficulty_selected_reset();
			lang_selected_reset();
			grid_maps_selected_reset();
			selected_game_mode = options_game_mode();
			selected = 0;
			break;
		}
		if (keys & KEY_A) {
			switch(choices[selected].choice) {
			case MENU_NEW_GAME:
				game_start();
				break;
			case MENU_LANGUAGE:
				lang_apply_selected();
				options_write();
				lang_selected_reset();
				break;
			case MENU_DIFFICULTY:
				if (options_difficulty_selected() != options_difficulty_current()) {
					dialog_type = DIALOG_CONFIRM;
					dialog_confirm_type = CONFIRM_DIFFICULTY;
					yes_selected = false;
				}
				break;
			case MENU_MODE:
				if (options_game_mode() != selected_game_mode) {
					dialog_type = DIALOG_CONFIRM;
					dialog_confirm_type = CONFIRM_MODE;
					yes_selected = false;
				}
				break;
			case MENU_EDITOR:
				game_editor_start();
				break;
			case MENU_OPTIONS:
				options_selected = true;
				selected = 0;
				break;
			case MENU_BACK:
				options_selected = false;
				options_difficulty_selected_reset();
				lang_selected_reset();
				grid_maps_selected_reset();
				selected_game_mode = options_game_mode();
				selected = 0;
				break;
			case MENU_GRID:
				if (strcmp(grid_maps_selected_name(), grid_maps_current_name()) != 0) {
					dialog_type = DIALOG_CONFIRM;
					dialog_confirm_type = CONFIRM_GRID;
					yes_selected = false;
				}
				break;
			case MENU_HIGHSCORES:
				dialog_type = DIALOG_HIGHSCORES;
				if (grid_maps_random(grid_maps_selected_name())) grid_maps_next();
				break;
			case MENU_HELP:
				help_page = 0;
				dialog_type = DIALOG_HELP;
				break;
			default:
				break;
			}
		}
		break;
	case DIALOG_HIGHSCORES:
		if (keys & (KEY_A | KEY_B)) {
			dialog_type = DIALOG_NORMAL;
			grid_maps_selected_reset();
		}
		if (keys & KEY_LEFT) {
			grid_maps_prev();
			if (grid_maps_random(grid_maps_selected_name())) grid_maps_prev();
		}
		if (keys & KEY_RIGHT) {
			grid_maps_next();
			if (grid_maps_random(grid_maps_selected_name())) grid_maps_next();
		}
		break;
	case DIALOG_CONFIRM:
		if (keys & (KEY_LEFT | KEY_RIGHT)) {
			yes_selected = !yes_selected;
		}
		if (keys & KEY_A) {
			dialog_type = DIALOG_NORMAL;
			if (yes_selected) {
				switch(dialog_confirm_type) {
				case CONFIRM_DIFFICULTY:
					options_difficulty_apply();
					break;
				case CONFIRM_MODE:
					options_set_game_mode(selected_game_mode);
					break;
				case CONFIRM_GRID:
					grid_maps_apply(grid_maps_selected_name());
					break;
				default:
					break;
				}
				options_write();
				game_start();
			} else {
				switch(dialog_confirm_type) {
				case CONFIRM_DIFFICULTY:
					options_difficulty_selected_reset();
					break;
				case CONFIRM_GRID:
					grid_maps_selected_reset();
					break;
				case CONFIRM_MODE:
					selected_game_mode = options_game_mode();
					break;
				default:
					break;
				}
			}
		}
		break;
	case DIALOG_HELP:
		if (keys & KEY_LEFT) {
			if (help_page == 0) {
				help_page = 2;
			} else {
				help_page--;
			}
		}
		if (keys & KEY_RIGHT) {
			help_page++;
			if (help_page == 3) {
				help_page = 0;
			}
		}
		if (keys & (KEY_A | KEY_B)) {
			dialog_type = DIALOG_NORMAL;
		}
		break;
	default:
		break;
	}
	return true;
}

void menu_init(void) {
	selected = 0;
	dialog_type = DIALOG_NORMAL;
	options_difficulty_selected_reset();
	lang_selected_reset();
	grid_maps_selected_reset();
	flying_sprite_init();
	logo_scale = 6.0f;
	logo_parallax = 0.0f;
	options_selected = false;
	selected_game_mode = options_game_mode();
}

void menu_close(void) {
	return;
}
