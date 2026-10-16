// intermission.c

#include <3ds.h>
#include "game.h"
#include "themes.h"
#include "text.h"

static char first_line_text[30];
static char second_line_text[30];
static float first_line_x;
static float first_line_y;
static float second_line_x;
static float second_line_y;
static bool done;

void intermission_init(const GameStats *stats) {
	if (!stats->dead) {
		snprintf(first_line_text,30,"SCORE: %d",stats->basescore);
		snprintf(second_line_text,30, "BONUS: %d",stats->clearbonus + stats->lifebonus);
		// FIX -- play some truimphant but not annoying sound
	} else {
		snprintf(first_line_text,30,"FINAL SCORE: %d",stats->basescore + stats->score);
		snprintf(second_line_text,30, "GAME OVER");
	}
	int first_line_w = text_getwidth(4,first_line_text);
	int second_line_w = text_getwidth(4,second_line_text);

	first_line_x=(WIDTH - first_line_w) / 2;
	first_line_y=HEIGHT;

	second_line_x=(WIDTH - second_line_w) / 2;
	second_line_y=HEIGHT;
	done = false;
}

void intermission_draw(void) {
	C2D_DrawRectSolid(BORDERLEFT, BORDERTOP, 0.4f, PLAYWIDTH, PLAYHEIGHT, C2D_Color32(0, 0, 0, 192));
	text_put(first_line_x, first_line_y, 0.5f, 4, theme.color.scorescrolltext, first_line_text);
	text_put(second_line_x, second_line_y, 0.5f, 4, theme.color.bonusscrolltext, second_line_text);
}

bool intermission_update(u32 keys) {
	if (keys & KEY_TOUCH) {
		done = true;
	}
	if (done) return false;
	if (first_line_y > 100) {
		first_line_y-=2;
	} else if (second_line_y > 130) {
		second_line_y-=2;
	}
	return true;
}
