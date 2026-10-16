// game.h
#pragma once

#include <3ds.h>
#include <citro2d.h>
#include <stdbool.h>
#include <stddef.h>

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
