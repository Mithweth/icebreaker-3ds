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
#include <dirent.h>
#include "game.h"
#include "themes.h"
#include "grid_maps.h"
#include "lang.h"
#include "text.h"

#define GRID_SPACING	2
#define GRID_MARGIN		4
#define CELL_SIZE		15
#define BLOCK_SIZE		(CELL_SIZE - GRID_SPACING)
#define GRID_WIDTH		(2 * GRID_MARGIN + GRID_MAPS_COLS * CELL_SIZE - GRID_SPACING)
#define GRID_HEIGHT		(2 * GRID_MARGIN + GRID_MAPS_ROWS * CELL_SIZE - GRID_SPACING)

typedef enum {
	EDITOR_CLEAR,
	EDITOR_LOAD,
	EDITOR_SAVE,
	EDITOR_BACK,
} EditorEntryType;

typedef enum {
	DIALOG_NONE,
	DIALOG_SAVE,
	DIALOG_LOAD,
	DIALOG_CONFIRM_DELETE_CURRENT,
	DIALOG_CONFIRM_DELETE,
	DIALOG_CONFIRM_SAVE,
	DIALOG_CONFIRM_SAVE_CURRENT,
	DIALOG_ERROR,
} EditorDialogType;

typedef enum {
	COMMENT_PLAIN,
	COMMENT_SAVE,
	COMMENT_LOAD,
	COMMENT_DELETE,
} EditorCommentType;

typedef enum {
	ERROR_MESSAGE,
	ERROR_MESSAGE_SAVE,
	ERROR_MESSAGE_MAX_GRIDS,
} EditorErrorMessage;

typedef struct {
	char *label;
	EditorEntryType type;
} EditorEntry;

static char grid[GRID_MAPS_COLS][GRID_MAPS_ROWS];
static u32 color_grid[GRID_MAPS_COLS][GRID_MAPS_ROWS];
static bool swipe_active;
static char current_char;
static EditorCommentType comment_type;
static EditorDialogType dialog_type;
static char grid_name[64];
static char* grids[MAX_SAVED_GRIDS + 1];
static EditorErrorMessage error_message;
static size_t grid_count;
static size_t current_grid;
static bool yes_selected;
static bool current_grid_changed;
static EditorEntry choices[] = { 
	{ "EDITOR_CLEAR", EDITOR_CLEAR },
	{ "EDITOR_LOAD", EDITOR_LOAD },
	{ "EDITOR_SAVE", EDITOR_SAVE },
	{ "EDITOR_BACK", EDITOR_BACK },
};

static int block_count(void) {
	size_t n = 0;
	for(size_t j = 0; j < GRID_MAPS_ROWS; j++) {
		for(size_t i = 0; i < GRID_MAPS_COLS; i++) {
			if (grid[i][j] == '#') n++;
		}
	}
	return n;
}

bool editor_current_grid_changed(void) {
	return current_grid_changed;
}

static size_t saved_grid_count(void) {
	size_t n = 0;
	for(size_t i = 0; i < grid_maps_count(); i++) {
		if (grid_maps_custom(grid_maps_get_name(i))) n++;
	}
	return n;
}

static char block_touched(touchPosition touch) {
	if (touch.px < GRID_MARGIN || touch.py < GRID_MARGIN) return ' ';
	int i = (touch.px - GRID_MARGIN) / CELL_SIZE;
	int j = (touch.py - GRID_MARGIN) / CELL_SIZE;
	if (i >= GRID_MAPS_COLS || j >= GRID_MAPS_ROWS) return ' ';
	grid[i][j] = (grid[i][j] == '#') ? '.' : '#';
	return grid[i][j];
}

static int menu_touched(touchPosition touch) {
	int buttons = sizeof(choices) / sizeof(choices[0]);
	if (touch.px < GRID_WIDTH + GRID_SPACING) return -1;
	int bh = (GRID_HEIGHT - (buttons - 1) * GRID_SPACING) / buttons;
	int i = (touch.py - GRID_SPACING) / bh;
	if (i >= buttons) return -1;
	return i;
}

static bool block_swipe(touchPosition touch, char c) {
	if (touch.px < GRID_MARGIN || touch.py < GRID_MARGIN) return false;
	int i = (touch.px - GRID_MARGIN) / CELL_SIZE;
	int j = (touch.py - GRID_MARGIN) / CELL_SIZE;
	if (i >= GRID_MAPS_COLS || j >= GRID_MAPS_ROWS) return false;
	grid[i][j] = c;
	return true;
}

static bool editor_save_write(void) {
	char filename[256];
	FILE *file;
	snprintf(filename, sizeof(filename), "%s/%s.ibg", CUSTOM_GRID_DIRECTORY, grid_name);
	file = fopen(filename, "w");
	if (!file) {
		printf("Can't open %s for writing\n", filename);
		return false;
	}
	for (size_t j = 0; j < GRID_MAPS_ROWS; j++) {
		for (size_t i = 0; i < GRID_MAPS_COLS; i++) {
			fputc(grid[i][j], file);
		}
		fputc('\n', file);
	}
	printf("%s: Grid saved successfully\n", filename);
	fclose(file);
	if (strcmp(grid_name, grid_maps_current_name()) == 0) current_grid_changed = true;
	return true;
}

static bool editor_save_read(const char *name) {
	char filename[256];
	FILE *file;
	snprintf(filename, sizeof(filename), "%s/%s.ibg", CUSTOM_GRID_DIRECTORY, name);
	file = fopen(filename, "r");
	if (!file) {
		printf("Can't open %s for reading\n", filename);
		return false;
	}
	for (size_t j = 0; j < GRID_MAPS_ROWS; j++) {
		for (size_t i = 0; i < GRID_MAPS_COLS; i++) {
			int c = fgetc(file);
			if (c != '#' && c != '.') {
				printf("%s:%zu: invalid character '%c'\n", filename, j, c);
				fclose(file);
				return false;
			}
			grid[i][j] = c;
		}
		int c = fgetc(file);
		if (c != '\n' && (j != GRID_MAPS_ROWS - 1 || c == EOF)) {
			printf("%s:%zu: file incomplete\n", filename, j);
			fclose(file);
			return false;
		}
	}
	printf("%s: Grid loaded successfully\n", filename);
	fclose(file);
	snprintf(grid_name, sizeof(grid_name), "%s", name);
	return true;
}

static int alphasortstringarray(const void *a, const void *b) {
	return strcmp(*(const char *const *)a, *(const char *const *)b);
}

static bool editor_saves_load(void) {
	DIR *dir = opendir(CUSTOM_GRID_DIRECTORY);
	if (!dir) return false;
	for (size_t i = 0; i < grid_count; i++) free(grids[i]);
	grid_count = 0;
	current_grid = 0;
	struct dirent *entry;
	while ((entry = readdir(dir))) {
		size_t len = strlen(entry->d_name);
		if (len <= 4 || strcmp(entry->d_name + len - 4, ".ibg") != 0) {
			continue;
		}
		if (grid_count >= MAX_SAVED_GRIDS) {
			printf("Too many grids in %s (max %d)\n", CUSTOM_GRID_DIRECTORY, MAX_SAVED_GRIDS);
			break;
		}

		char *grid_name = malloc(len - 3);
		if (!grid_name) {
			closedir(dir);
			return false;
		}

		memcpy(grid_name, entry->d_name, len - 4);
		grid_name[len - 4] = '\0';
		grids[grid_count++] = grid_name;
		printf("Found grid: %s\n", grid_name);
	}

	closedir(dir);
	qsort(grids, grid_count, sizeof(grids[0]), alphasortstringarray);
	printf("%s: Found %zu grids\n", CUSTOM_GRID_DIRECTORY, grid_count);
	return true;
}

static bool editor_save_delete(const char* name) {
	char filename[256];
	snprintf(filename, sizeof(filename), "%s/%s.ibg", CUSTOM_GRID_DIRECTORY, name);
	if (remove(filename) != 0) {
		printf("%s: cannot delete\n", filename);
		return false;
	}
	printf("%s: deleted successfully\n", filename);
	if (strcmp(name, grid_maps_current_name()) == 0) current_grid_changed = true;
	return true;
}

static SwkbdCallbackResult grid_name_filter(void *user, const char **message, const char *text, size_t textlen) {
    (void)user;
    for (size_t i = 0; i < textlen; i++) {
        char c = text[i];
        if (!((c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z') || (c >= '0' && c <= '9') || c == '_')) {
            *message = lang_get("SWKB_ERROR_MESSAGE");
            return SWKBD_CALLBACK_CONTINUE;
        }
    }
    return SWKBD_CALLBACK_OK;
}

static bool dialog_save_prompt(void) {
	SwkbdState keyboard;
	char new_grid_name[sizeof(grid_name)];
	swkbdInit(&keyboard, SWKBD_TYPE_NORMAL, 2, 12);
	swkbdSetInitialText(&keyboard, grid_name);
	swkbdSetValidation(&keyboard, SWKBD_NOTEMPTY_NOTBLANK, SWKBD_FILTER_CALLBACK, 0);
	swkbdSetFilterCallback(&keyboard, grid_name_filter, NULL);
	swkbdSetButton(&keyboard, SWKBD_BUTTON_LEFT, lang_get("SWKB_CANCEL"), false);
	swkbdSetButton(&keyboard, SWKBD_BUTTON_RIGHT, lang_get("SWKB_OK"), true);
	if (swkbdInputText(&keyboard, new_grid_name, sizeof(new_grid_name)) != SWKBD_BUTTON_RIGHT) return false;
	strcpy(grid_name, new_grid_name);
	return true;
}

bool dialog_update(u32 keys, touchPosition touch) {
	if (keys & KEY_B) {
		return true;
	}
	if ((keys & KEY_A) && !yes_selected) {
		return true;
	}

	if (keys & (KEY_LEFT | KEY_RIGHT)) yes_selected = !yes_selected;

	if ((keys & KEY_TOUCH) && touch.px >= 60 && touch.px <= 110 && touch.py >= 160 && touch.py <= 178) {
		return true;
	}
	switch(dialog_type) {
	case DIALOG_LOAD:
		if (keys & KEY_SELECT) {
			if (strcmp(grid_maps_current_name(), grids[current_grid]) == 0) {
				dialog_type = DIALOG_CONFIRM_DELETE_CURRENT;
			} else {
				dialog_type = DIALOG_CONFIRM_DELETE;
			}
			yes_selected = false;
			return false;
		}
		if (keys & KEY_UP) {
			if (current_grid == 0) {
				current_grid = grid_count - 1;
			} else {
				current_grid--;
			}
			return false;
		}
		if (keys & KEY_DOWN) {
			current_grid++;
			if (current_grid == grid_count) {
				current_grid = 0;
			}
			return false;
		}
		if ((keys & KEY_A) || ((keys & KEY_TOUCH) && touch.px >= 210 && touch.px <= 260 && touch.py >= 160 && touch.py <= 178)) {
			if (!editor_save_read(grids[current_grid])) {
				dialog_type = DIALOG_ERROR;
				error_message = ERROR_MESSAGE;
				return false;
			}
			comment_type = COMMENT_LOAD;
			return true;
		}
		break;
	case DIALOG_CONFIRM_SAVE_CURRENT:
	case DIALOG_CONFIRM_SAVE:
		if ((keys & KEY_A) || ((keys & KEY_TOUCH) && touch.px >= 210 && touch.px <= 260 && touch.py >= 160 && touch.py <= 178)) {
			if (!editor_save_write()) {
				dialog_type = DIALOG_ERROR;
				error_message = ERROR_MESSAGE;
				return false;
			}
			grid_maps_load();
			comment_type = COMMENT_SAVE;
			return true;
		}
		break;		
	case DIALOG_SAVE:
		if ((keys & KEY_TOUCH) && touch.px >= 60 && touch.px <= 260 && touch.py >= 102 && touch.py <= 123) {
			dialog_save_prompt();
			return false;
		}
		if ((keys & KEY_A) || ((keys & KEY_TOUCH) && touch.px >= 210 && touch.px <= 260 && touch.py >= 160 && touch.py <= 178)) {
			if (grid_maps_get_index(grid_name) != -1) {
				if (grid_maps_custom(grid_name)) {
					if (strcmp(grid_maps_current_name(), grid_name) == 0) {
						dialog_type = DIALOG_CONFIRM_SAVE_CURRENT;
					} else {
						dialog_type = DIALOG_CONFIRM_SAVE;
					}
				} else {
					dialog_type = DIALOG_ERROR;
					error_message = ERROR_MESSAGE_SAVE;
				}

				yes_selected = false;
				return false;
			}
			if (saved_grid_count() >= MAX_SAVED_GRIDS) {
				dialog_type = DIALOG_ERROR;
				error_message = ERROR_MESSAGE_MAX_GRIDS;
				return false;
			}
			if (!editor_save_write()) {
				dialog_type = DIALOG_ERROR;
				error_message = ERROR_MESSAGE;
				return false;
			}
			grid_maps_load();
			comment_type = COMMENT_SAVE;
			return true;
		}
		break;
	case DIALOG_CONFIRM_DELETE_CURRENT:
	case DIALOG_CONFIRM_DELETE:
		if ((keys & KEY_A) || ((keys & KEY_TOUCH) && touch.px >= 210 && touch.px <= 260 && touch.py >= 160 && touch.py <= 178)) {
			if (!editor_save_delete(grids[current_grid])) {
				dialog_type = DIALOG_ERROR;
				error_message = ERROR_MESSAGE;
				return false;
			}
			comment_type = COMMENT_DELETE;
			grid_maps_load();
			if (!editor_saves_load()) {
				dialog_type = DIALOG_ERROR;
				error_message = ERROR_MESSAGE;
				return false;
			}
			if (grid_count == 0) return true;
			dialog_type = DIALOG_LOAD;
			return false;
		}
		break;
	default:
		break;
	}
	return false;
}

static void dialog_error_draw(void) {
	C2D_DrawRectSolid(0, 0, 0.4f, WIDTH, HEIGHT, C2D_Color32(0, 0, 0, 192));
	C2D_DrawRectSolid(50.0f, 50.0f, 0.41f, 220.0f, 140.0f, theme.color.menuhighlight);
	C2D_DrawRectSolid(52.0f, 52.0f, 0.42f, 216.0f, 136.0f, theme.color.background);
	text_cput(160.0f, 60.0f, 0.43f, 3, theme.color.gameovertext, lang_get("EDITOR_ERROR"));
	switch(error_message) {
	case ERROR_MESSAGE:
		text_put(60.0f, 100.0f, 0.43f, 2, theme.color.normaltext, lang_get("ERROR_MESSAGE"));
		break;
	case ERROR_MESSAGE_SAVE:
		text_put(60.0f, 100.0f, 0.43f, 2, theme.color.normaltext, lang_get("ERROR_MESSAGE_SAVE_1"));
		text_put(60.0f, 122.0f, 0.43f, 2, theme.color.normaltext, lang_get("ERROR_MESSAGE_SAVE_2"));
		text_put(60.0f, 144.0f, 0.43f, 2, theme.color.normaltext, lang_get("ERROR_MESSAGE_SAVE_3"));
		break;
	case ERROR_MESSAGE_MAX_GRIDS:
		text_put(60.0f, 100.0f, 0.43f, 2, theme.color.normaltext, lang_get("ERROR_MESSAGE_MAX_GRIDS_1"));
		text_put(60.0f, 122.0f, 0.43f, 2, theme.color.normaltext, lang_get("ERROR_MESSAGE_MAX_GRIDS_2"));
		text_put(60.0f, 144.0f, 0.43f, 2, theme.color.normaltext, lang_get("ERROR_MESSAGE_MAX_GRIDS_3"));
		break;
	default:
		break;
	}
}

static void dialog_draw(void) {
	C2D_DrawRectSolid(0, 0, 0.4f, WIDTH, HEIGHT, C2D_Color32(0, 0, 0, 192));
	C2D_DrawRectSolid(50.0f, 50.0f, 0.41f, 220.0f, 140.0f, theme.color.menuhighlight);
	C2D_DrawRectSolid(52.0f, 52.0f, 0.42f, 216.0f, 136.0f, theme.color.background);
	int box_ok = 20.0f + text_getwidth(3, lang_get("CHOICE_OK"));
	int box_cancel = 20.0f + text_getwidth(3, lang_get("CHOICE_CANCEL"));
	if (yes_selected) {
		text_cput(60 + box_cancel / 2, 162, 0.44f, 3, theme.color.menuhighlight, lang_get("CHOICE_CANCEL"));
		C2D_DrawRectSolid(260.0f - box_ok, 160.0f, 0.43f, box_ok, 18.0f, theme.color.menuhighlight);
		text_cput(260.0f - box_ok + box_ok / 2, 162, 0.44f, 3, theme.color.background, lang_get("CHOICE_OK"));
	} else {
		C2D_DrawRectSolid(60.0f, 160.0f, 0.43f, box_cancel, 18.0f, theme.color.menuhighlight);
		text_cput(60 + box_cancel / 2, 162, 0.44f, 3, theme.color.background, lang_get("CHOICE_CANCEL"));
		text_cput(200 + box_ok / 2, 162, 0.44f, 3, theme.color.menuhighlight, lang_get("CHOICE_OK"));		
	}
	switch(dialog_type) {
	case DIALOG_SAVE:
		text_put(60.0f, 60.0f, 0.43f, 2, theme.color.normaltext, lang_get("EDITOR_PROMPT"));
		C2D_DrawRectSolid(60.0f, 102.0f, 0.43f, 200.0f, 21.0f, theme.color.textentrybox);
		text_put(70, 104, 0.44f, 3, theme.color.textentrytext, grid_name);
		break;
	case DIALOG_LOAD:
		text_put(60.0f, 60.0f, 0.43f, 2, theme.color.normaltext, lang_get("EDITOR_PROMPT"));
		text_put(70, 104, 0.44f, 3, theme.color.normaltext, grids[current_grid]);
		break;
	case DIALOG_CONFIRM_DELETE_CURRENT:
		text_put(60.0f, 60.0f, 0.43f, 2, theme.color.normaltext, lang_get("EDITOR_CONFIRM_CURRENT_DELETE_1"));
		text_put(60.0f, 82.0f, 0.43f, 2, theme.color.normaltext, lang_get("EDITOR_CONFIRM_CURRENT_DELETE_2"));
		text_put(60.0f, 104.0f, 0.43f, 2, theme.color.normaltext, lang_get("EDITOR_CONFIRM_CURRENT_DELETE_3"));
		break;
	case DIALOG_CONFIRM_DELETE:
		text_put(60.0f, 60.0f, 0.43f, 2, theme.color.normaltext, lang_get("EDITOR_CONFIRM_DELETE_1"));
		text_put(60.0f, 82.0f, 0.43f, 2, theme.color.normaltext, lang_get("EDITOR_CONFIRM_DELETE_2"));
		text_put(60.0f, 104.0f, 0.43f, 2, theme.color.normaltext, lang_get("EDITOR_CONFIRM_DELETE_3"));
		break;
	case DIALOG_CONFIRM_SAVE:
		text_put(60.0f, 60.0f, 0.43f, 2, theme.color.normaltext, lang_get("EDITOR_CONFIRM_SAVE_1"));
		text_put(60.0f, 82.0f, 0.43f, 2, theme.color.normaltext, lang_get("EDITOR_CONFIRM_SAVE_2"));
		text_put(60.0f, 104.0f, 0.43f, 2, theme.color.normaltext, lang_get("EDITOR_CONFIRM_SAVE_3"));
		break;
	case DIALOG_CONFIRM_SAVE_CURRENT:
		text_put(60.0f, 60.0f, 0.43f, 2, theme.color.normaltext, lang_get("EDITOR_CONFIRM_CURRENT_SAVE_1"));
		text_put(60.0f, 82.0f, 0.43f, 2, theme.color.normaltext, lang_get("EDITOR_CONFIRM_CURRENT_SAVE_2"));
		text_put(60.0f, 104.0f, 0.43f, 2, theme.color.normaltext, lang_get("EDITOR_CONFIRM_CURRENT_SAVE_3"));
		break;
	default:
		break;
	}
}

static void editor_reset(void) {
	for(size_t j = 0; j < GRID_MAPS_ROWS; j++) {
		for(size_t i = 0; i < GRID_MAPS_COLS; i++) {
			grid[i][j] = '.';
			u8 r = (random() % (theme.color.boardfillmaxr - theme.color.boardfillminr + 1)) + theme.color.boardfillminr;
			u8 g, b;

			if (theme.color.boardfillminr == theme.color.boardfillming && theme.color.boardfillmaxr == theme.color.boardfillmaxg) {
				g = r;
			} else {
				g = (random() % (theme.color.boardfillmaxg - theme.color.boardfillming + 1)) + theme.color.boardfillming;
			}

			if (theme.color.boardfillminr == theme.color.boardfillminb && theme.color.boardfillmaxr == theme.color.boardfillmaxb) {
				b = r;
			} else if (theme.color.boardfillming == theme.color.boardfillminb && theme.color.boardfillmaxg == theme.color.boardfillmaxb) {
				b = g;
			} else {
				b = (random() % (theme.color.boardfillmaxb - theme.color.boardfillminb + 1)) + theme.color.boardfillminb;
			}
			color_grid[i][j] = C2D_Color32(r, g, b, 255);
		}
	}
}

void editor_init(void) {
	editor_reset();
	snprintf(grid_name, sizeof(grid_name), "%s", lang_get("EDITOR_UNTITLED"));
	swipe_active = false;
	dialog_type = DIALOG_NONE;
	comment_type = COMMENT_PLAIN;
	yes_selected = false;
	current_grid_changed = false;
	error_message = ERROR_MESSAGE;
}

bool editor_update(u32 keys, touchPosition touch) {
	switch(dialog_type) {
	case DIALOG_NONE:
		if (keys) comment_type = COMMENT_PLAIN;
		if (keys & KEY_START) return false;
		if (keys & KEY_TOUCH) {
			int choice = menu_touched(touch);
			if (choice > -1) {
				switch(choices[choice].type) {
				case EDITOR_BACK:
					return false;
				case EDITOR_LOAD:
					if (!editor_saves_load()) {
						dialog_type = DIALOG_ERROR;
						error_message = ERROR_MESSAGE;
						break;
					}
					if (grid_count > 0) {
						dialog_type = DIALOG_LOAD;
						yes_selected = false;
					}
					break;
				case EDITOR_SAVE:
					if (block_count() != 0) {
						dialog_type = DIALOG_SAVE;
						yes_selected = false;
					}
					break;
				case EDITOR_CLEAR:
					editor_reset();
					break;
				default:
					break;
				}
				return true;
			} else {
				current_char = block_touched(touch);
				if (current_char != ' ') swipe_active = true;
			}
		}
		if (swipe_active && (hidKeysHeld() & KEY_TOUCH)) block_swipe(touch, current_char);
		if (swipe_active && hidKeysUp() & KEY_TOUCH) swipe_active = false;
		break;
	case DIALOG_ERROR:
		if (keys) {
			dialog_type = DIALOG_NONE;
		}
		break;
	case DIALOG_SAVE:
	case DIALOG_CONFIRM_DELETE_CURRENT:
	case DIALOG_CONFIRM_SAVE_CURRENT:
	case DIALOG_CONFIRM_SAVE:
	case DIALOG_CONFIRM_DELETE:
	case DIALOG_LOAD:
		if (dialog_update(keys, touch)) {
			dialog_type = DIALOG_NONE;
		}
		break;
	default:
		break;
	}
	return true;
}

void editor_draw(void) {
	C2D_DrawRectSolid(0, 0, 0.0f, WIDTH, HEIGHT, theme.color.background);
	C2D_DrawRectSolid(0, 0, 0.01f, GRID_WIDTH, GRID_HEIGHT, theme.color.progressbaroutline);
	C2D_DrawRectSolid(2, 2, 0.02f, GRID_WIDTH - GRID_MARGIN, GRID_HEIGHT - GRID_MARGIN, theme.color.background);

	for(int j = 0; j < GRID_MAPS_ROWS; j++) {
		for(int i = 0; i < GRID_MAPS_COLS; i++) {
			if (grid[i][j] == '.') continue;
			C2D_DrawRectSolid(GRID_MARGIN + i * CELL_SIZE, GRID_MARGIN + j * CELL_SIZE, 0.1f, BLOCK_SIZE, BLOCK_SIZE, color_grid[i][j]);
		}
	}
	size_t buttons = sizeof(choices) / sizeof(choices[0]);
	int bh = (GRID_HEIGHT - (buttons - 1) * GRID_SPACING) / buttons;
	int bw = WIDTH - GRID_WIDTH - GRID_SPACING;
	for(size_t i = 0; i < buttons; i++) {
		C2D_DrawRectSolid(GRID_WIDTH + GRID_SPACING, i * (bh + GRID_SPACING), 0.01f, bw, bh, theme.color.progressbaroutline);
		C2D_DrawRectSolid(GRID_WIDTH + GRID_SPACING + GRID_SPACING, i * (bh + GRID_SPACING) + GRID_SPACING, 0.02f, bw - 2 * GRID_SPACING, bh - 2 * GRID_SPACING, theme.color.background);
		text_cput(GRID_WIDTH + GRID_SPACING + bw / 2, i * (bh + GRID_SPACING) + bh / 2 - 5.0f, 0.03f, 2, theme.color.normaltext, lang_get(choices[i].label));
	}
	C2D_DrawRectSolid(0, GRID_HEIGHT + GRID_SPACING, 0.01f, WIDTH, HEIGHT - GRID_HEIGHT - GRID_SPACING, theme.color.textentrytext);
	text_put(10.0f, GRID_HEIGHT + GRID_SPACING + 5.0f, 0.02f, 2, theme.color.normaltext, grid_name);
	char blocks[8];
	snprintf(blocks, sizeof(blocks), "%zu/%d", block_count(), GRID_MAPS_COLS * GRID_MAPS_ROWS);
	text_rput(310.0f, GRID_HEIGHT + GRID_SPACING + 5.0f, 0.02f, 3, theme.color.normaltext, blocks);
	char comment[100];
	switch(comment_type) {
	case COMMENT_PLAIN:
		snprintf(comment, sizeof(comment), "%s", lang_get("EDITOR_COMMENT"));
		break;
	case COMMENT_SAVE:
		snprintf(comment, sizeof(comment), "%s", lang_get("EDITOR_COMMENT_SAVE"));
		break;
	case COMMENT_LOAD:
		snprintf(comment, sizeof(comment), "%s", lang_get("EDITOR_COMMENT_LOAD"));
		break;
	case COMMENT_DELETE:
		snprintf(comment, sizeof(comment), "%s", lang_get("EDITOR_COMMENT_DELETE"));
		break;
	default:
		break;
	}
	text_put(10.0f, HEIGHT - 15.0f, 0.02f, 1, theme.color.normaltext, comment);
	
	switch(dialog_type) {
	case DIALOG_ERROR:
		dialog_error_draw();
		break;
	case DIALOG_SAVE:
	case DIALOG_CONFIRM_DELETE_CURRENT:
	case DIALOG_CONFIRM_SAVE_CURRENT:
	case DIALOG_CONFIRM_SAVE:
	case DIALOG_CONFIRM_DELETE:
	case DIALOG_LOAD:
		dialog_draw();
		break;
	default:
		break;
	}
}
void editor_close(void) {
	return;
}