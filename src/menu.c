// menu.c
#include "menu.h"

#include <3ds.h>
#include <citro2d.h>
#include <string.h>
#include <math.h>
#include "themes.h"
#include "game.h"
#include "text.h"

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

				C2D_DrawRectangle(x + (i * 6 + start) * scale, y + row * scale, 0.5f, (col - start) * scale, scale, gradient[row], gradient[row], gradient[row + 1], gradient[row + 1]);
				for (int c = start; c < col; c++) {
				    if (row == 0 || !(logo_glyphs[glyph][row - 1] & (1 << (4 - c)))) {
				        C2D_DrawRectSolid(x + (i * 6 + c) * scale, y + row * scale, 0.5f, scale, 1.5f, C2D_Color32(245, 252, 255, 230));
				    }
				}
			}
		}
	}
}

static void menu_draw_rounded_rect(float x, float y, float w, float h, u32 color) {
    if (w <= 0 || h <= 0) return;

    float r = fminf(w / 2.0f, h / 2.0f);

    C2D_DrawRectSolid(x + r, y + h / 2.0f - r, 0.5f, w - 2.0f * r, 2.0f * r, color);
    C2D_DrawCircleSolid(x + r, y + h / 2.0f, 0.5f, r, color);
    C2D_DrawCircleSolid(x + w - r, y + h / 2.0f, 0.5f, r, color);
}

static void progress_bar_draw(int pct) {
    if (pct < 0) pct = 0;
    if (pct > 100) pct = 100;
    menu_draw_rounded_rect(10.0f, 220.0f, 380.0f, 14.0f, color.progressbaroutline);
    menu_draw_rounded_rect(12.0f, 222.0f, 376.0f, 10.0f, C2D_Color32(20, 50, 75, 255));
    menu_draw_rounded_rect(12.0f, 222.0f, (376.0f) * pct / 100.0f, 10.0f, color.progressbarinner);
}

void menu_init(void) {
	return;
}
void menu_draw(const GameStats *stats) {
	C2D_DrawRectSolid(0, 0, 0.0f, 400.0f, 240.0f, color.background);
	logo_drawtext("ICE", 148, 55, 6);
	logo_drawtext("BREAKER", 77, 110, 6);
	char tmptext[30]; // should be plenty big.
	text_put(175, 10, 2, color.normaltext, "SCORE");
	text_put(10, 10, 2, color.normaltext, "LEVEL");
	text_put(280, 10, 2, color.normaltext, "HIGH SCORE");
	text_put(10, 200, 2, color.normaltext, "LIVES");
	snprintf(tmptext,30,"%03d %%",stats->clear);
	text_put(340, 200, 2, color.normaltext, tmptext);
	snprintf(tmptext,30,"%05d",stats->score + stats->basescore);
	text_put(175, 25, 2, color.normaltext, tmptext);
	snprintf(tmptext,30,"%02d",stats->level);
	text_put(10, 25, 2, color.normaltext, tmptext);
	snprintf(tmptext,30,"%05d",stats->highscore);
	text_put(330, 25, 2, color.normaltext, tmptext);
	for(int i = 0; i < stats->lives; i++) {
		C2D_DrawImageAt(spriteimage, 80.0f + (16.0f * i), 195.0f, 0.5f, NULL, 1.0f, 1.0f);
	}
	progress_bar_draw(stats->clear);
}
bool menu_update(u32 keys, touchPosition touch) {

}
void menu_close(void) {
	return;
}
