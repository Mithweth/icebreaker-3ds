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

#define GRID_MAPS_ROWS 12
#define GRID_MAPS_COLS 16

static char* grids[MAX_GRIDS];
static size_t grid_count;
static size_t current_grid;
static size_t selected_grid;

static int alphasortstringarray(const void *a, const void *b) {
	return strcmp(*(const char *const *)a, *(const char *const *)b);
}

bool grid_maps_apply(const char* gridname) {
	int potential_grid = grid_maps_get_index(gridname);
	if (potential_grid < 0) {
		printf("Error: unknown grid %s. Reverse to default\n", gridname);
		grid_reset(' ');
		return false;
	}

	char line[GRID_MAPS_COLS + 3];
	char gridfilename[256];
	char tmpgrid[GRID_MAPS_COLS][GRID_MAPS_ROWS];
	size_t current_row = 0;
	snprintf(gridfilename, 256, "%s/%s.ibg", GRID_DIRECTORY, gridname);
	FILE* file = fopen(gridfilename, "r");
	if (!file) {
		printf("Error: Can't open grid file %s. Reverse to default\n", gridfilename);
		grid_reset(' ');
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
			grid_reset(' ');
			return false;
		}
		for (int i = 0; i < GRID_MAPS_COLS; i++) {
			if (line[i] != '.' && line[i] != '#') {
				printf("%s:%zu: only '.' and '#' are supported. Reverse to default\n", gridfilename, current_row + 1);
				grid_reset(' ');
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
		grid_reset(' ');
		return false;
	}
	if (!grid_expand(GRID_MAPS_COLS, GRID_MAPS_ROWS, tmpgrid)) {
		printf("%s: No valid ground has been found in that grid. Reverse to default\n", gridfilename);
		grid_reset(' ');
		return false;
	}
	current_grid = potential_grid;
	printf("grid loaded successfully: %s\n", gridname);
	return true;
}

bool grid_maps_load(void) {
	DIR *dir = opendir(GRID_DIRECTORY);
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

		if (grid_count >= MAX_GRIDS) {
			printf("Too many grids in %s (max %d)\n", GRID_DIRECTORY, MAX_GRIDS);
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
	printf("%s: Found %zu grids\n", GRID_DIRECTORY, grid_count);
	if (grid_count == 0) {
		return false;
	}
	return true;
}

void grid_maps_next(void) {
	if (grid_count == 0) {
		return;
	}
	selected_grid++;
	if (selected_grid == grid_count) {
		selected_grid = 0;
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
}

void grid_maps_selected_reset(void) {
	selected_grid = current_grid;
}

const char* grid_maps_get_name(size_t id) {
	if (id >= grid_count) {
		return NULL;
	}
	return grids[id];
}

int grid_maps_get_index(const char *name) {
	for (size_t i = 0; i < grid_count; i++) {
		if (strcmp(grids[i], name) == 0) return i;
	}
	return -1;
}

const char* grid_maps_selected_name(void) {
	return grid_maps_get_name(selected_grid);
}

const char* grid_maps_current_name(void) {
	return grid_maps_get_name(current_grid);
}

size_t grid_maps_get_count(void) {
	return grid_count;
}
