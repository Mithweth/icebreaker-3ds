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
#include <stddef.h>

bool grid_maps_load(void);
void grid_maps_next(void);
void grid_maps_prev(void);
void grid_maps_selected_reset(void);
bool grid_maps_apply(const char* name);
const char* grid_maps_selected_name(void);
const char* grid_maps_current_name(void);
bool grid_maps_random(const char *name);
size_t grid_maps_get_count(void);
const char* grid_maps_get_name(size_t id);
int grid_maps_get_index(const char *name);
