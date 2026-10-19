/*
 * IceBreaker
 * Copyright (c) 2000-2020 Matthew Miller <mattdm@mattdm.org>
 * Copyright (c) 2026 Jean-Baptiste Langlois
 *
 * Original game by Matthew Miller.
 * Nintendo 3DS port by Jean-Baptiste Langlois (2026).
 *
 * <http://www.mattdm.org/icebreaker/>
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
#include <stdlib.h>
#include "grid.h"
#include "game.h"
#include "themes.h"
#include "line.h"

static long rcount;
#define MAXRCOUNT 80000

char grid[WIDTH][HEIGHT];
char maskgrid[WIDTH][HEIGHT];
static u32 block_colors[COLS][ROWS];

char grid_at(int x, int y) {
	if (x < 0 || x >= WIDTH || y < 0 || y >= HEIGHT)
		return 'X';
	return grid[x][y];
}

static void grid_squarefill(int x, int y)
{
	// x and y must be the top left corner of a square, or else this
	// will look silly. and there's no bounds checking!

	if (grid_at(x,y)==' ' || grid_at(x,y)=='1' || grid_at(x,y)=='2' || grid_at(x,y)=='w')
	{
		grid_mark(x,y,BLOCKWIDTH,BLOCKHEIGHT,'.');
		grid_squarefill(x+BLOCKWIDTH, y);
		grid_squarefill(x-BLOCKWIDTH, y);
		grid_squarefill(x, y+BLOCKHEIGHT);
		grid_squarefill(x, y-BLOCKHEIGHT);
	}
}

long grid_countcleared(void) {
	int i, j;
	long c;
	c=0;
	for (i=0;i<WIDTH;i++)
		for (j=0;j<HEIGHT;j++)
			if (grid_at(i,j) == ' ' || grid_at(i,j) == '*')
				c++;
	return c;
}

void grid_reset(char c) {
	for (int x=0;x<WIDTH;x++) {
		for (int y=0;y<HEIGHT;y++)
		{
			grid[x][y]=c;
		}
	}
}

void grid_mark(int x, int y, int w, int h, char fillchar) {
	int i;
	
	// Optimizing this routine much further seems about impossible to me
	// but if you know how to do it, let me know. This is by far the
	// most frequently-called function in the whole game. Short of
	// rethinking the collision detection -- which could stand a rethink
	// anyway -- there's not much that can be done.
	
	if (x < 0) { w -= - x; x = 0; }
	if (y < 0)  { h -= - y;  y = 0; }
	if (x + w > WIDTH)  w = WIDTH - x;
	if (y + h > HEIGHT) h = HEIGHT - y;
	if (w <= 0 || h <= 0) return;
	for (i=x;i<x+w;i++)
		memset(&grid[i][y],fillchar,h);
}

void grid_generate_colors(void) {
	for (int x = 0; x < COLS; x++) {
		for (int y = 0; y < ROWS; y++) {
			int r = (random() % (theme.color.boardfillmaxr - theme.color.boardfillminr + 1)) + theme.color.boardfillminr;
			int g, b;

			if (theme.color.boardfillminr == theme.color.boardfillming && theme.color.boardfillmaxr == theme.color.boardfillmaxg)
				g = r;
			else
				g = (random() % (theme.color.boardfillmaxg - theme.color.boardfillming + 1)) + theme.color.boardfillming;

			if (theme.color.boardfillminr == theme.color.boardfillminb && theme.color.boardfillmaxr == theme.color.boardfillmaxb)
				b = r;
			else if (theme.color.boardfillming == theme.color.boardfillminb && theme.color.boardfillmaxg == theme.color.boardfillmaxb)
				b = g;
			else
				b = (random() % (theme.color.boardfillmaxb - theme.color.boardfillminb + 1)) + theme.color.boardfillminb;

			block_colors[x][y] = C2D_Color32(r, g, b, 255);
		}
	}
}

void grid_drawblocks(void) {
	C2D_DrawRectSolid(0, 0, 0.0f, WIDTH, HEIGHT, theme.color.background);

	for (int bx = 0; bx < COLS; bx++) {
		for (int by = 0; by < ROWS; by++) {
			int x = bx * BLOCKWIDTH;
			int y = by * BLOCKHEIGHT;

			if (grid_at(x,y) == '.' || grid_at(x,y) == '-' || grid_at(x,y) == '|')
				continue;
			C2D_DrawRectSolid(x, y, 0.1f, BLOCKWIDTH, BLOCKHEIGHT, theme.color.gridline);

			if (theme.color.gridhighlight == theme.color.gridline) {
				C2D_DrawRectSolid(x, y, 0.2f, BLOCKWIDTH - 1, BLOCKHEIGHT - 1, block_colors[bx][by]);
			} else {
				C2D_DrawRectSolid(x, y, 0.2f, BLOCKWIDTH - 1, BLOCKHEIGHT - 1, theme.color.gridhighlight);
				C2D_DrawRectSolid(x + 1, y + 1, 0.3f, BLOCKWIDTH - 2, BLOCKHEIGHT - 2, block_colors[bx][by]);
			}
		}
	}
}

static int grid_penguinsearch(int i, int j)
{
	int searchval=0;
	if (i < 0 || i >= WIDTH || j < 0 || j >= HEIGHT)
		return 0;
	rcount++; 
	// kludge! FIX! BAD!
	if (rcount>MAXRCOUNT) // bail
	{
		fprintf(stderr,"Damn. Ran out of recursions.\n");
		return(2);
	}

		  
	if (maskgrid[i][j]==' '
		|| maskgrid[i][j]=='1' || maskgrid[i][j]=='2' || maskgrid[i][j]=='w')  // Ah ha! The nefarious "instant melting ice" bug solved!  NOTE: if more lines are added to the game, add them here too!
	{
		maskgrid[i][j]=',';
		
		// hmmm. the "ice-shelf-collapse" bug *isn't* fixed. :(
		
		searchval=grid_penguinsearch(i+BLOCKWIDTH, j);
		if (!searchval) searchval=grid_penguinsearch(i-BLOCKWIDTH, j);
		if (!searchval) searchval=grid_penguinsearch(i, j-BLOCKHEIGHT);
		if (!searchval) searchval=grid_penguinsearch(i, j+BLOCKHEIGHT);
		
	}
	else if (maskgrid[i][j]=='*') // found a penguin!
	{
		searchval=1;	
	}
	return searchval;
}

void grid_checkempty(int x, int y)
{
	// if square isn't empty, just return....
	if (grid_at(x,y)!=' ') return;

	// it'd be nice to find a way to keep this longer...
	memcpy(maskgrid,grid,WIDTH*HEIGHT);
	
	// grid_penguinsearch at that spot...
	rcount=0;
	if (!grid_penguinsearch(x,y)) // area is clear!
	{
		// this makes sure x and y are the top left corners of blocks.
		// since the area is empty of penguins, it should be completely
		// safe to use this isntead of grid_floodfill here. really. :)
		grid_squarefill(x / BLOCKWIDTH * BLOCKWIDTH, y / BLOCKHEIGHT * BLOCKHEIGHT);
	}
}
