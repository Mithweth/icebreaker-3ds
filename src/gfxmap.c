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

static C2D_SpriteSheet sheet;
static GfxImageIndex *indexes;  // owned, count entries
static size_t count;

// Callers pass the short PNG name; tex3ds names the macros gfx_<name>_idx
// because every header is generated from a file named gfx.t3s.
static int gfxmap_get_index(const char *name) {
	char index_name[256];
	snprintf(index_name, sizeof(index_name), "gfx_%s_idx", name);
	for (size_t i = 0; i < count; i++) {
		if (strcmp(indexes[i].name, index_name) == 0) {
			return indexes[i].index;
		}
	}
	return -1;
}

C2D_Image gfxmap_get_image(const char *name) {
	int index = gfxmap_get_index(name);

	if (index < 0) {
		printf("Image not found: %s\n", name);
		return (C2D_Image){0};
	}
	return C2D_SpriteSheetGetImage(sheet, index);
}

// Fill indexes from tex3ds header `filename`. On failure the table
// is freed and left empty, never half-filled.
static bool gfxmap_load(const char *filename) {
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

		if (count == capacity) {
			size_t new_capacity = capacity ? capacity * 2 : 32;
			GfxImageIndex *grown = realloc(indexes, new_capacity * sizeof(*grown));
			if (!grown) {
				printf("Out of memory loading %s\n", filename);
				free(indexes);
				indexes = NULL;
				count = 0;
				fclose(f);
				return false;
			}
			indexes = grown;
			capacity = new_capacity;
		}

		GfxImageIndex *entry = &indexes[count++];

		strcpy(entry->name, name);
		entry->index = atoi(value);
	}
	printf("%s: Loaded %zu images\n", filename, count);
	fclose(f);
	return true;
}

static bool gfxmap_load_assets(const char *path) {
	char filename[256];

	snprintf(filename, sizeof(filename), "%s/gfx.t3x", path);

	sheet = C2D_SpriteSheetLoad(filename);
	if (!sheet) {
		printf("Cannot load spritesheet: %s\n", filename);
		return false;
	}

	snprintf(filename, sizeof(filename), "%s/gfx.h", path);

	if (!gfxmap_load(filename)) {
		return false;
	}

	return true;
}

bool gfxmap_assets_loaded() {
	return sheet != NULL;
}

void gfxmap_close(void) {
	if (sheet) {
		C2D_SpriteSheetFree(sheet);
	}
	sheet = NULL;
	free(indexes);
	indexes = NULL;
	count = 0;
}

bool gfxmap_init(void) {
	if (!gfxmap_load_assets("romfs:/bitmaps")) {
		printf("Cannot load assets\n");
		gfxmap_close();
		return false;
	}
	return true;
}
