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

#include <citro2d.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "gfxmap.h"
#include "game.h"

static GfxSheet builtin;
static GfxSheet custom;

// Callers pass the short PNG name; tex3ds names the macros <prefix>_<name>_idx
// because every header is generated from a file named gfx.t3s.
static int gfxmap_get_index(GfxSheet *gfx_sheet, const char *name) {
	char index_name[256];
	snprintf(index_name, sizeof(index_name), "%s_%s_idx", gfx_sheet->prefix, name);
	for (size_t i = 0; i < gfx_sheet->count; i++) {
		if (strcmp(gfx_sheet->indexes[i].name, index_name) == 0) {
			return gfx_sheet->indexes[i].index;
		}
	}
	return -1;
}

C2D_Image gfxmap_get_image(const char *name) {
	int index = gfxmap_get_index(&custom, name);
	if (index >= 0) {
		return C2D_SpriteSheetGetImage(custom.sheet, index);
	}
	index = gfxmap_get_index(&builtin, name);
	if (index < 0) {
		printf("Image not found: %s\n", name);
		return (C2D_Image){0};
	}
	return C2D_SpriteSheetGetImage(builtin.sheet, index);
}

// Fill indexes from tex3ds header `filename`. On failure the table
// is freed and left empty, never half-filled.
static bool gfxmap_load(GfxSheet* gfx_sheet, const char *filename) {
	FILE *f = fopen(filename, "r");
	if (!f) {
		printf("Cannot open %s\n", filename);
		return false;
	}

	size_t capacity = 0;
	char line[256];

	while (fgets(line, sizeof(line), f)) {
		char directive[32];
		char name[GFX_NAME_MAX];
		char value[32];

		if (sscanf(line, "%31s %95s %31s", directive, name, value) != 3) {
			continue;
		}

		if (strcmp(directive, "#define") != 0) {
			continue;
		}

		size_t len = strlen(name);

		if (len < 4 || strcmp(name + len - 4, "_idx") != 0) {
			continue;
		}

		if (gfx_sheet->count == capacity) {
			size_t new_capacity = capacity ? capacity * 2 : 32;
			GfxImageIndex *grown = realloc(gfx_sheet->indexes, new_capacity * sizeof(*grown));
			if (!grown) {
				printf("Out of memory loading %s\n", filename);
				free(gfx_sheet->indexes);
				gfx_sheet->indexes = NULL;
				gfx_sheet->count = 0;
				fclose(f);
				return false;
			}
			gfx_sheet->indexes = grown;
			capacity = new_capacity;
		}

		GfxImageIndex *entry = &gfx_sheet->indexes[gfx_sheet->count++];

		strcpy(entry->name, name);
		entry->index = atoi(value);
	}
	printf("%s: Loaded %zu images\n", filename, gfx_sheet->count);
	fclose(f);
	return true;
}

static bool gfxmap_load_assets(GfxSheet *gfx_sheet, const char *path, const char *prefix) {
	char filename[256];
	snprintf(filename, sizeof(filename), "%s/%s.t3x", path, prefix);

	gfx_sheet->sheet = C2D_SpriteSheetLoad(filename);
	if (!gfx_sheet->sheet) {
		printf("Cannot load spritesheet: %s\n", filename);
		return false;
	}

	snprintf(filename, sizeof(filename), "%s/%s.h", path, prefix);
	snprintf(gfx_sheet->prefix, sizeof(gfx_sheet->prefix), "%s", prefix);
	if (!gfxmap_load(gfx_sheet, filename)) {
		return false;
	}

	return true;
}

static void gfxmap_free_sheet(GfxSheet* gfx_sheet) {
	if (gfx_sheet->sheet) {
		C2D_SpriteSheetFree(gfx_sheet->sheet);
	}
	gfx_sheet->sheet = NULL;
	free(gfx_sheet->indexes);
	gfx_sheet->indexes = NULL;
	gfx_sheet->count = 0;
}

void gfxmap_close(void) {
	gfxmap_free_sheet(&custom);
	gfxmap_free_sheet(&builtin);
}

void gfxmap_custom_close(void) {
	gfxmap_free_sheet(&custom);
}

bool gfxmap_custom_load(const char *prefix) {
	if (!gfxmap_load_assets(&custom, CUSTOM_THEME_DIRECTORY, prefix)) {
		printf("Cannot load assets\n");
		gfxmap_free_sheet(&custom);
		return false;
	}
	return true;
}

bool gfxmap_init(void) {
	if (!gfxmap_load_assets(&builtin, "romfs:/bitmaps", "gfx")) {
		printf("Cannot load assets\n");
		gfxmap_free_sheet(&builtin);
		return false;
	}
	return true;
}
