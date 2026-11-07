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

#include <3ds.h>
#include <citro3d.h>
#include <stdbool.h>

typedef enum {
	TRANSITION_SPLIT,
	TRANSITION_EXPLODE_OUT,
	TRANSITION_EXPLODE_IN,
	TRANSITION_FLIP,
	TRANSITION_COUNT
} TransitionType;

void transition_init(TransitionType type);
void transition_draw(C3D_RenderTarget *target);
bool transition_active(void);
bool transition_hides_screen(void);
bool transition_update(void);
void transition_close(void);
