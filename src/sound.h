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

#pragma once

#include <stdbool.h>

typedef enum {
	SNDOUCH,
	SNDBREAK,
	SNDVICTORY,
	SNDDEFEAT,
	SNDFREEZE,
	SNDSPEED,
	SNDSHIELD,
	SNDSHIELDOFF,
	SNDCOUNT
} Sound;

bool sound_load(Sound id, const char *soundname);
bool sound_play(Sound id);
void sound_init(void);
void sound_reset(void);
void sound_close(void);
