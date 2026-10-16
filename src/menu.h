// menu.h

#pragma once

#include <3ds.h>
#include "game.h"

void menu_init(void);
void menu_draw(bool pause, const GameStats *stats);
bool menu_update(u32 keys);
void menu_close(void);
void menu_bottom_draw(bool pause);
