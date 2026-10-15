// grid.h
#pragma once

#include "constants.h"

void grid_mark(int x, int y, int w, int h, char fillchar);
void grid_drawblocks(void);
void grid_generate_colors(void);
void grid_checkempty(int x, int y);
char grid_at(int x, int y);
void grid_reset(char c);

extern char grid[WIDTH][HEIGHT];
extern char maskgrid[WIDTH][HEIGHT];
