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

#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <stdbool.h>
#include <string.h>
#include <dirent.h>
#include "grid.h"
#include "grid_maps.h"
#include "game.h"

#define RANDOM_HISTORY	3

typedef enum {
	GRID_BUILTIN,
	GRID_CUSTOM,
} GridType;

typedef struct {
	char *filename;
	char *name;
	GridType type;
} GridFile;

static GridFile grids[MAX_GRIDS + 1];
static size_t grid_count;
static size_t current_grid;
static size_t selected_grid;
static size_t random_history[RANDOM_HISTORY];
static size_t random_history_count;
static char tmpgrid[GRID_MAPS_COLS][GRID_MAPS_ROWS];

static bool grid_in_history(size_t grid_number) {
	for (size_t i = 0; i < random_history_count; i++) {
		if (random_history[i] == grid_number) return true;
	}
	return false;
}

static size_t random_pick(void) {
	size_t chosen_grid = random() % (grid_count - 1);
	while (random_history_count < grid_count - 1 && grid_in_history(chosen_grid)) {
		chosen_grid = random() % (grid_count - 1);
	}
	if (random_history_count == RANDOM_HISTORY) {
		for (size_t i = 1; i < random_history_count; i++) random_history[i - 1] = random_history[i];
		random_history_count--;
	}
	random_history[random_history_count++] = chosen_grid;
	return chosen_grid;
}

static int alphasortstringarray(const void *a, const void *b) {
	const char* x = ((const GridFile *)a)->name;
	const char* y = ((const GridFile *)b)->name;
	return strcmp(x, y);
}

const char* grid_maps_get_name(size_t id) {
	if (id >= grid_count) {
		return NULL;
	}
	return grids[id].name;
}

const char* grid_maps_get_filename(size_t id) {
	if (id >= grid_count) {
		return NULL;
	}
	return grids[id].filename;
}

int grid_maps_get_index(const char *name) {
	for (size_t i = 0; i < grid_count; i++) {
		if (strcmp(grids[i].name, name) == 0) return i;
	}
	return -1;
}

bool grid_maps_set_current_name(const char *name) {
	int id = grid_maps_get_index(name);
	if (id < 0) return false;
	current_grid = id;
	return true;
}

const char* grid_maps_selected_name(void) {
	return grid_maps_get_name(selected_grid);
}

const char* grid_maps_current_name(void) {
	return grid_maps_get_name(current_grid);
}

bool grid_maps_random(const char *name) {
	return strcmp("random", name) == 0;
}

size_t grid_maps_count(void) {
	return grid_count;
}

bool grid_maps_custom(const char *name) {
	for (size_t i = 0; i < grid_count; i++) {
		if (strcmp(grids[i].name, name) == 0) {
			return (grids[i].type == GRID_CUSTOM);
		}
	}
	return false;
}

char grid_maps_selected_at(int x, int y) {
	return tmpgrid[x][y];
}

static bool load_grid(size_t grid_index) {
	char line[GRID_MAPS_COLS + 3];
	char gridfilename[256];
	size_t current_row = 0;
	snprintf(gridfilename, 256, "%s", grid_maps_get_filename(grid_index));
	FILE* file = fopen(gridfilename, "r");
	if (!file) {
		printf("Error: Can't open grid file %s. Reverse to default\n", gridfilename);
		return false;
	}

	while(current_row < GRID_MAPS_ROWS && fgets(line,sizeof(line),file)) {
		if (line[0] == '\0') {
			break;
		}
		size_t len = strcspn(line, "\r\n");
		if (len != GRID_MAPS_COLS) {
			printf("%s:%zu: invalid grid row length\n", gridfilename, current_row + 1);
			fclose(file);
			return false;
		}
		for (int i = 0; i < GRID_MAPS_COLS; i++) {
			if (line[i] != '.' && line[i] != '#') {
				printf("%s:%zu: only '.' and '#' are supported. Reverse to default\n", gridfilename, current_row + 1);
				fclose(file);
				return false;
			}
			tmpgrid[i][current_row] = line[i];
		}
		current_row++;
	}
	fclose(file);
	if (current_row < GRID_MAPS_ROWS) {
		printf("%s: the grid file is not complete. Reverse to default\n", gridfilename);
		return false;
	}
	return true;
}

bool grid_maps_apply(const char* gridname) {
	const char* grid_name = gridname;
	bool random_mode = false;
	if (grid_maps_random(gridname)) {
		if (grid_count < 2) {
			printf("Error: random mode needs at least 2 grids. Reverse to default\n");
			grid_reset();
			return false;
		}
		random_mode = true;
		grid_name = grid_maps_get_name(random_pick());
	}
	int potential_grid = grid_maps_get_index(grid_name);
	if (potential_grid < 0) {
		printf("Error: unknown grid %s. Reverse to default\n", grid_name);
		grid_reset();
		return false;
	}

	if (!load_grid(potential_grid)) {
		grid_reset();
		return false;
	}
	if (!grid_expand(GRID_MAPS_COLS, GRID_MAPS_ROWS, tmpgrid)) {
		printf("%s: No valid ground has been found in that grid. Reverse to default\n", gridname);
		grid_reset();
		return false;
	}
	current_grid = (random_mode) ? grid_count - 1 : (size_t)potential_grid;
	printf("grid loaded successfully: %s\n", grid_name);
	return true;
}

static bool grid_maps_load_from_directory(const char* dirname, GridType type) {
	DIR *dir = opendir(dirname);
	if (!dir) return false;
	size_t dirlen = strlen(dirname);
	struct dirent *entry;
	while ((entry = readdir(dir))) {
		size_t len = strlen(entry->d_name);
		if (len <= 4 || strcmp(entry->d_name + len - 4, ".ibg") != 0) {
			continue;
		}

		if (grid_count >= MAX_GRIDS) {
			printf("Too many grids in %s (max %d)\n", dirname, MAX_GRIDS);
			break;
		}

		char *grid_name = malloc(len - 3);
		if (!grid_name) {
			closedir(dir);
			return false;
		}
		memcpy(grid_name, entry->d_name, len - 4);
		grid_name[len - 4] = '\0';
		if (grid_maps_get_index(grid_name) != -1) {
			printf("Warning: the grid %s already exists. Skipping\n", grid_name);
			free(grid_name);
			continue;
		}
		char *file_name = malloc(len + dirlen + 2);
		if (!file_name) {
			free(grid_name);
			closedir(dir);
			return false;
		}
		snprintf(file_name, len + dirlen + 2, "%s/%s", dirname, entry->d_name);
		grids[grid_count].name = grid_name;
		grids[grid_count].type = type;
		grids[grid_count++].filename = file_name;
		printf("Found grid: %s\n", grid_name);
	}
	closedir(dir);
	return true;
}

bool grid_maps_load(void) {
	char current_name[64] = "";
	if (current_grid < grid_count) {
		snprintf(current_name, sizeof(current_name), "%s", grids[current_grid].name);
	}
	for (size_t i = 0; i < grid_count; i++) {
		free(grids[i].name);
		free(grids[i].filename);
	}
	grid_count = 0;
	current_grid = 0;
	random_history_count = 0;

	if (!grid_maps_load_from_directory(GRID_DIRECTORY, GRID_BUILTIN)) return false;
	if (!grid_maps_load_from_directory(CUSTOM_GRID_DIRECTORY, GRID_CUSTOM)) {
		printf("Warning: cannot load custom grids\n");
	}

	qsort(grids, grid_count, sizeof(grids[0]), alphasortstringarray);
	printf("Found %zu grids\n", grid_count);

	if (grid_count == 0) {
		return false;
	}
	if (grid_count > 1) {
		grids[grid_count].name = strdup("random");
		grids[grid_count++].filename = NULL;
	}
	if (!grid_maps_set_current_name(current_name)) {
		grid_maps_set_current_name("classic");
	}
	grid_maps_selected_reset();
	return true;
}

void grid_maps_selected_reset(void) {
	selected_grid = current_grid;
	if (!grid_maps_random(grid_maps_selected_name())) {
		load_grid(selected_grid);
	}
}

void grid_maps_next(void) {
	if (grid_count == 0) {
		return;
	}
	selected_grid++;
	if (selected_grid == grid_count) {
		selected_grid = 0;
	}
	if (!grid_maps_random(grid_maps_selected_name())) {
		load_grid(selected_grid);
	}
}

void grid_maps_prev(void) {
	if (grid_count == 0) {
		return;
	}
	if (selected_grid == 0) {
		selected_grid = grid_count - 1;
	} else {
		selected_grid--;
	}
	if (!grid_maps_random(grid_maps_selected_name())) {
		load_grid(selected_grid);
	}
}
