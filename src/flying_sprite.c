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

#include <citro2d.h>
#include <math.h>
#include "themes.h"

typedef enum {
	SPRITE_NORMAL,
	SPRITE_FALLING,
	SPRITE_CELEBRATING,
} FlyingSpriteStatus;

typedef struct {
	float angle;
	float x;
	float y;
	float z;
	float boost;
	float speed;
	float progress;
	FlyingSpriteStatus status;
	float fall_speed;
	C2D_Image image;
} FlyingSprite;

static FlyingSprite sprite;
static float initial_z;

void flying_sprite_init(void) {
	sprite.image = theme.spriteimage;
	sprite.speed = 0.015f;
	sprite.boost = 0.0f;
	sprite.status = SPRITE_NORMAL;
	sprite.fall_speed = 0.0f;
}

void flying_sprite_gameover(void) {
    sprite.status = SPRITE_FALLING;
    sprite.fall_speed = 0.0f;
}

void flying_sprite_victory(void) {
    sprite.status = SPRITE_CELEBRATING;
    sprite.progress = 0.0f;
    initial_z = sprite.z;
}

void flying_sprite_capture(void) {
	sprite.boost = sprite.speed > 0.0f ? 0.06f : -0.06f;
}

void flying_sprite_death(void) {
	sprite.speed = -sprite.speed;
	sprite.boost = sprite.speed > 0.0f ? 0.04f : -0.04f;
}

void flying_sprite_watch(void) {
	sprite.boost = -sprite.speed * 0.8f;
}

void flying_sprite_update(void) {
	switch(sprite.status) {
	case SPRITE_NORMAL:
		sprite.angle += sprite.speed + sprite.boost;
		sprite.boost *= 0.98f;

		if (sprite.angle >= 2.0f * M_PI) sprite.angle -= 2.0f * M_PI;
		if (sprite.angle < 0.0f) sprite.angle += 2.0f * M_PI;

		sprite.x = 200.0f + 150.0f * cosf(sprite.angle);
		sprite.y = 120.0f + 75.0f * sinf(sprite.angle);
		sprite.z = sinf(sprite.angle);
		break;
	case SPRITE_FALLING:
		sprite.fall_speed += 0.15f;
		sprite.y += sprite.fall_speed;
		break;
	case SPRITE_CELEBRATING:
	    sprite.progress += 1.0f / 45.0f;
	    if (sprite.progress >= 1.0f) {
	        sprite.status = SPRITE_NORMAL;
	        sprite.z = initial_z;
	    } else {
	        float jump = sinf(M_PI * sprite.progress);
	        sprite.z = initial_z + (1.5f - initial_z) * jump;
	    }
		break;
	default:
		break;
	}
}

void flying_sprite_draw(float parallax) {
	float scale = exp2f(sprite.z) * 0.75f;
	float x = sprite.x + sprite.z * parallax - (sprite.image.subtex->width / 2.0f) * scale;
	float y = sprite.y - (sprite.image.subtex->height / 2.0f) * scale;
	C2D_DrawImageAt(sprite.image, x, y, 0.4f + sprite.z * 0.2f, NULL, scale, scale);
}