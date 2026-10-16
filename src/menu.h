// menu.h

#pragma once

#include <3ds.h>
#include "game.h"

void menu_init(void);
void menu_draw(const GameStats *stats);
bool menu_update(u32 keys, touchPosition touch);
void menu_close(void);
