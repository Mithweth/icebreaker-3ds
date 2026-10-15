// grid.c

#include <3ds.h>
#include <citro2d.h>
#include <stdlib.h>
#include "grid.h"
#include "constants.h"
#include "themes.h"
#include "line.h"

static long rcount;
#define MAXRCOUNT 80000

char grid[WIDTH][HEIGHT];
char maskgrid[WIDTH][HEIGHT];
static u32 block_colors[COLS][ROWS];

char grid_at(int x, int y) {
	if (x < BORDERLEFT || x >= BORDERRIGHT || y < BORDERTOP || y >= BORDERBOTTOM)
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
	
	if (x < BORDERLEFT) { w -= BORDERLEFT - x; x = BORDERLEFT; }
	if (y < BORDERTOP)  { h -= BORDERTOP - y;  y = BORDERTOP; }
	if (x + w > BORDERRIGHT)  w = BORDERRIGHT - x;
	if (y + h > BORDERBOTTOM) h = BORDERBOTTOM - y;
	if (w <= 0 || h <= 0) return;
	for (i=x;i<x+w;i++)
		memset(&grid[i][y],fillchar,h);
}

void grid_generate_colors(void) {
	for (int x = 0; x < COLS; x++) {
		for (int y = 0; y < ROWS; y++) {
			int r = (random() % (color.boardfillmaxr - color.boardfillminr + 1)) + color.boardfillminr;
			int g, b;

			if (color.boardfillminr == color.boardfillming && color.boardfillmaxr == color.boardfillmaxg)
				g = r;
			else
				g = (random() % (color.boardfillmaxg - color.boardfillming + 1)) + color.boardfillming;

			if (color.boardfillminr == color.boardfillminb && color.boardfillmaxr == color.boardfillmaxb)
				b = r;
			else if (color.boardfillming == color.boardfillminb && color.boardfillmaxg == color.boardfillmaxb)
				b = g;
			else
				b = (random() % (color.boardfillmaxb - color.boardfillminb + 1)) + color.boardfillminb;

			block_colors[x][y] = C2D_Color32(r, g, b, 255);
		}
	}
}

void grid_drawblocks(void) {
	C2D_DrawRectSolid(BORDERLEFT, BORDERTOP, 0.0f, PLAYWIDTH, PLAYHEIGHT, color.background);

	for (int bx = 0; bx < COLS; bx++) {
		for (int by = 0; by < ROWS; by++) {
			int x = BORDERLEFT + bx * BLOCKWIDTH;
			int y = BORDERTOP + by * BLOCKHEIGHT;

			if (grid_at(x,y) == '.' || grid_at(x,y) == '-' || grid_at(x,y) == '|')
				continue;
			C2D_DrawRectSolid(x, y, 0.1f, BLOCKWIDTH, BLOCKHEIGHT, color.gridline);

			if (color.gridhighlight == color.gridline) {
				C2D_DrawRectSolid(x, y, 0.2f, BLOCKWIDTH - 1, BLOCKHEIGHT - 1, block_colors[bx][by]);
			} else {
				C2D_DrawRectSolid(x, y, 0.2f, BLOCKWIDTH - 1, BLOCKHEIGHT - 1, color.gridhighlight);
				C2D_DrawRectSolid(x + 1, y + 1, 0.3f, BLOCKWIDTH - 2, BLOCKHEIGHT - 2, block_colors[bx][by]);
			}
		}
	}
}

static int grid_penguinsearch(int i, int j)
{
	int searchval=0;
	if (i < BORDERLEFT || i >= BORDERRIGHT || j < BORDERTOP || j >= BORDERBOTTOM)
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
		grid_squarefill( (((x-BORDERLEFT)/BLOCKWIDTH ) * BLOCKWIDTH ) +BORDERLEFT, (((y-BORDERTOP)/BLOCKHEIGHT) * BLOCKHEIGHT) +BORDERTOP);
	}
}
