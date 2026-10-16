// intermission.h
#pragma once

#include <3ds.h>
#include "game.h"

bool intermission_init(const GameStats *stats);
void intermission_draw(void);
bool intermission_update(u32 keys);
