/*
 * IceBreaker
 * Copyright (c) 2000-2020 Matthew Miller <mattdm@mattdm.org>
 * Copyright (c) 2000-2020 Enrico Tassi <gareuselesinge@infinito.it>
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

#pragma once

typedef enum { EASY, NORMAL, HARD } GameDifficultyType;

void options_difficulty_next(void);
void options_difficulty_prev(void);
void options_difficulty_apply(void);
GameDifficultyType options_difficulty_current(void);
GameDifficultyType options_difficulty_selected(void);
const char* options_difficulty_serialize(GameDifficultyType t);
GameDifficultyType options_difficulty_deserialize(const char* s);
void options_difficulty_selected_reset(void);
bool options_write(void);
bool options_read(void);
