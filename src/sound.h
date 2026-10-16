// sound.h

#pragma once

typedef enum {
	SNDOUCH,
	SNDBREAK,
	SNDVICTORY,
	SNDCOUNT
} Sound;

bool sound_load(Sound id, const char *soundname);
bool sound_play(Sound id);
void sound_init(void);
void sound_reset(void);
void sound_close(void);
