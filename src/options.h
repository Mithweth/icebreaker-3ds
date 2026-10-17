// options.h
#pragma once

typedef enum { EASY, NORMAL, HARD } GameDifficultyType;

typedef struct {
	GameDifficultyType difficulty;
} GameOptionsType;

void options_difficulty_next(void);
void options_difficulty_prev(void);
void options_difficulty_apply(void);
void options_difficulty_set(GameDifficultyType diff);
GameDifficultyType options_difficulty_current(void);
GameDifficultyType options_difficulty_selected(void);
const char* options_difficulty_serialize(GameDifficultyType t);
GameDifficultyType options_difficulty_deserialize(const char* s);
