// menu.c
#include "menu.h"

#include <3ds.h>
#include <citro2d.h>
#include <string.h>
#include <math.h>
#include "themes.h"
#include "game.h"
#include "text.h"
#include "options.h"
#include "grid.h"
#include "hiscore.h"

typedef enum { DIALOG_NORMAL, DIALOG_CONFIRM, DIALOG_HIGHSCORES, DIALOG_HELP } DialogType;
typedef enum { MENU_NEW_GAME, MENU_DIFFICULTY, MENU_THEME, MENU_HIGHSCORES, MENU_HELP } MenuChoice;

typedef struct {
	const char *label;
	MenuChoice choice;
} MenuEntry;

static DialogType dialog_type;
static bool yes_selected;
static const char *logo_chars = "ICEBRAK";
static size_t selected;
static int hiscore_scroll;
static int hiscore_direction;
static MenuEntry choices[] = {
	{"NEW GAME", MENU_NEW_GAME},
	{"DIFFICULTY", MENU_DIFFICULTY},
	{"THEME", MENU_THEME},
	{"HIGH SCORES", MENU_HIGHSCORES},
	{"HELP", MENU_HELP},
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

static void logo_drawtext(const char *text, float x, float y, float scale) {
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
	C2D_DrawRectSolid(x + r, y + h / 2.0f - r, 0.3f, w - 2.0f * r, 2.0f * r, color);
	C2D_DrawCircleSolid(x + r, y + h / 2.0f, 0.3f, r, color);
	C2D_DrawCircleSolid(x + w - r, y + h / 2.0f, 0.3f, r, color);
}

static void progress_bar_draw(int pct) {
	if (pct < 0) pct = 0;
	if (pct > 100) pct = 100;
	menu_draw_rounded_rect(10.0f, 220.0f, 380.0f, 14.0f, theme.color.progressbaroutline);
	menu_draw_rounded_rect(12.0f, 222.0f, 376.0f, 10.0f, theme.color.progressbarinner);
	menu_draw_rounded_rect(12.0f, 222.0f, (376.0f) * pct / 100.0f, 10.0f, theme.color.progressbarfill);
}

void menu_init(void) {
	selected = 0;
	dialog_type = DIALOG_NORMAL;
}

void menu_bottom_draw(bool pause) {
	if (pause) {
		C2D_DrawRectSolid(BORDERLEFT, BORDERTOP, 0.9f, PLAYWIDTH, PLAYHEIGHT, C2D_Color32(0, 0, 0, 192));
		text_put(100, 110, 1.0f, 4, theme.color.normaltext, "PAUSE");
	}
}

void menu_draw(bool pause, const GameStats *stats) {
	C2D_DrawRectSolid(0, 0, 0.0f, 400.0f, 240.0f, theme.color.background);
	char tmptext[30]; // should be plenty big.
	text_put(175, 10, 0.3f, 2, theme.color.normaltext, "SCORE");
	text_put(10, 10, 0.3f, 2, theme.color.normaltext, "LEVEL");
	text_put(280, 10, 0.3f, 2, theme.color.normaltext, "HIGH SCORE");
	text_put(10, 200, 0.3f, 2, theme.color.normaltext, "LIVES");
	snprintf(tmptext,30,"%d %%",stats->clear);
	text_rput(390, 200, 0.3f, 2, theme.color.normaltext, tmptext);
	snprintf(tmptext,30,"%d",stats->score + stats->basescore);
	text_cput(200, 25, 0.3f, 2, theme.color.normaltext, tmptext);
	snprintf(tmptext,30,"%d",stats->level);
	text_put(20, 25, 0.3f, 2, theme.color.normaltext, tmptext);
	snprintf(tmptext,30,"%d",stats->highscore);
	text_rput(385, 25, 0.3f, 2, theme.color.normaltext, tmptext);
	for(int i = 0; i < stats->lives; i++) {
		C2D_DrawImageAt(theme.spriteimage, 80.0f + (16.0f * i), 195.0f, 0.3f, NULL, 1.0f, 1.0f);
	}
	progress_bar_draw(stats->clear);
	if (!pause) {
		logo_drawtext("ICE", 148, 55, 6);
		logo_drawtext("BREAKER", 77, 110, 6);
		return;
	}
	C2D_DrawRectSolid(50.0f, 50.0f, 0.3f, 300.0f, 140.0f, theme.color.menuhighlight);
	C2D_DrawRectSolid(52.0f, 52.0f, 0.31f, 296.0f, 136.0f, theme.color.background);
	switch(dialog_type) {
	case DIALOG_NORMAL:
		for(size_t i = 0; i < sizeof(choices) / sizeof(choices[0]); i++) {
			u32 option_color = theme.color.normaltext;
			if (selected == i) {
				C2D_DrawRectSolid(50.0f, 56 + 25 * i, 0.35f, 300.0f, 23.0f, theme.color.menuhighlight);
				option_color = theme.color.background;
			}
			text_put(60, 61 + 25 * i, 0.4f, 3, option_color, choices[i].label);
			if (choices[i].choice == MENU_DIFFICULTY) {
				text_rput(340, 61 + 25 * i, 0.4f, 3, option_color, options_difficulty_serialize(options_difficulty_selected()));
			}
			if (choices[i].choice == MENU_THEME) {
				text_rput(340, 61 + 25 * i, 0.4f, 3, option_color, themes_current_name());
			}
		}
		break;
	case DIALOG_CONFIRM:
		text_put(60, 60, 0.4f, 3, theme.color.normaltext, "Changing difficulty");
		text_put(60, 82, 0.4f, 3, theme.color.normaltext, "requires restart.");
		text_put(60, 104, 0.4f, 3, theme.color.normaltext, "End this game?");
		if (yes_selected) {
			C2D_DrawRectSolid(70.0f, 145, 0.35f, 80.0f, 23.0f, theme.color.menuhighlight);
			text_put(80, 150, 0.4f, 3, theme.color.background, "YES");
			text_put(275, 150, 0.4f, 3, theme.color.normaltext, "NO");
		} else {
			C2D_DrawRectSolid(250.0f, 145, 0.35f, 80.0f, 23.0f, theme.color.menuhighlight);
			text_put(80, 150, 0.4f, 3, theme.color.normaltext, "YES");
			text_put(275, 150, 0.4f, 3, theme.color.background, "NO");
		}
		break;
	case DIALOG_HIGHSCORES:
		text_cput(200, 60, 0.4f, 3, theme.color.bonusscrolltext, "HIGH SCORES");
		C2D_Flush();
		// C3D_* invert x and y axis and use the left bottom as the origin
		// so 60 means Y from bottom, so (240 - 60) for C2D_*
		// so 160 means Y from bottom, so (240 - 160) for C2D_*
		// 0 and 400 are axis
		C3D_SetScissor(GPU_SCISSOR_NORMAL, 60, 0, 160, 400);
		for(int i = 0; i < HISCORENUM; i++) {
			int y = 100 + 20 * i - hiscore_scroll;
			HighScore* user = hiscore_get(i);
			char nb[4];
			snprintf(nb, 4, "%d.", i + 1);
			char score[10];
			snprintf(score, 10, "%ld", user->score);
			text_put(60, y, 0.4f, 2, theme.color.normaltext, nb);
			text_put(100, y, 0.4f, 2, theme.color.normaltext, user->name);
			text_rput(340, y, 0.4f, 2, theme.color.normaltext, score);
		}
		C2D_Flush();
		C3D_SetScissor(GPU_SCISSOR_DISABLE, 0, 0, 0, 0);
		break;
	default:
		break;
	}
}

bool menu_update(u32 keys) {
	switch(dialog_type) {
	case DIALOG_NORMAL:
		if ((keys & KEY_UP) && (selected > 0)) {
			selected--;
		}
		if ((keys & KEY_DOWN) && (selected < (sizeof(choices) / sizeof(choices[0])) - 1)) {
			selected++;
		}
		if (keys & KEY_LEFT) {
			switch(selected) {
			case MENU_DIFFICULTY:
				options_difficulty_prev();
				break;
			case MENU_THEME:
				themes_prev();
				break;
			default:
				break;
			}
		}
		if (keys & KEY_RIGHT) {
			switch(selected) {
			case MENU_DIFFICULTY:
				options_difficulty_next();
				break;
			case MENU_THEME:
				themes_next();
				break;
			default:
				break;
			}
		}
		if (keys & KEY_A) {
			printf("Selected option: %zu\n", selected);
			switch(selected) {
			case MENU_DIFFICULTY:
				dialog_type = DIALOG_CONFIRM;
				yes_selected = false;
				break;
			case MENU_THEME:
				themes_apply();
				grid_generate_colors();
				break;
			case MENU_HIGHSCORES:
				dialog_type = DIALOG_HIGHSCORES;
				hiscore_scroll = 0;
				hiscore_direction = 1;
				break;
			default:
				break;
			}
		}
		break;
	case DIALOG_HIGHSCORES:
		if (keys & KEY_A) {
			dialog_type = DIALOG_NORMAL;
		}
		hiscore_scroll += hiscore_direction;

		if (hiscore_scroll >= 140) {
			hiscore_scroll = 140;
			hiscore_direction = -1;
		} else if (hiscore_scroll <= 0) {
			hiscore_scroll = 0;
			hiscore_direction = 1;
		}
		break;
	case DIALOG_CONFIRM:
		if (keys & (KEY_LEFT | KEY_RIGHT)) {
			yes_selected = !yes_selected;
		}
		if (keys & KEY_A) {
			if (yes_selected) {
				dialog_type = DIALOG_NORMAL;
				options_difficulty_apply();
				game_start();
				return true;
			}
			dialog_type = DIALOG_NORMAL;
		}
		break;
	default:
		break;
	}
	return true;
}
void menu_close(void) {
	return;
}
