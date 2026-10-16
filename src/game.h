// game.h
#pragma once

#include <3ds.h>
#include <citro2d.h>
#include <stdbool.h>
#include <stddef.h>

#define MAXPENGUINS 100
#define PENGUINSPEED 2
#define SPRITE_SIZE 0.75f

#define BLOCKWIDTH 10
#define BLOCKHEIGHT 10

#define COLS 32
#define ROWS 24

#define PLAYWIDTH (COLS*BLOCKWIDTH)
#define PLAYHEIGHT (ROWS*BLOCKHEIGHT)

#define MARGINTOP 0
#define MARGINBOTTOM 0
#define MARGINLEFT 0
#define MARGINRIGHT 0

#define PLAYWIDTH (COLS*BLOCKWIDTH)
#define PLAYHEIGHT (ROWS*BLOCKHEIGHT)

#define WIDTH (MARGINLEFT+PLAYWIDTH+MARGINRIGHT)
#define HEIGHT (MARGINTOP+PLAYHEIGHT+MARGINBOTTOM)

#define BORDERTOP    MARGINTOP
#define BORDERBOTTOM (MARGINTOP+PLAYHEIGHT)
#define BORDERLEFT   MARGINLEFT
#define BORDERRIGHT  (MARGINLEFT+PLAYWIDTH)
#define LINEMAXSTUCK 750

#define PERCENTREQUIRED 80
#define PERCENTBONUS 80
#define PERCENTEXTRABONUS 85

#define MAX_THEMES	16

typedef struct {
    int basescore;
    int highscore;
    int score;
    int level;
    int clearbonus;
    int lifebonus;
    int lives;
    int clear;
    bool dead;
} GameStats;

bool game_init(void);
bool game_update(u32 keys, touchPosition touch);
void game_draw(C3D_RenderTarget *top, C3D_RenderTarget *bottom);
void game_close(void);
