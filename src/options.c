// options.c
#include <string.h>
#include <stdio.h>
#include "options.h"

static GameOptionsType options = {.difficulty = NORMAL};

static GameDifficultyType selected_difficulty;

void options_difficulty_next(void) {
    if (selected_difficulty == HARD) {
        selected_difficulty = EASY;
    } else {
        selected_difficulty++;
    }
}

void options_difficulty_prev(void) {
    if (selected_difficulty == EASY) {
        selected_difficulty = HARD;
    } else {
        selected_difficulty--;
    }
}

void options_difficulty_apply(void) {
    options.difficulty = selected_difficulty;
    printf("selected difficulty: %s\n", options_difficulty_serialize(options.difficulty));
}

void options_difficulty_set(GameDifficultyType diff) {
    selected_difficulty = diff;
    options.difficulty = diff;
}

GameDifficultyType options_difficulty_current(void) {
    return options.difficulty;
}


GameDifficultyType options_difficulty_selected(void) {
    return selected_difficulty;
}

const char* options_difficulty_serialize(GameDifficultyType t) {
	switch(t) {
	case EASY:
		return "easy";
	case NORMAL:
		return "normal";
	case HARD:
		return "hard";
	default:
		return NULL;
	}
}

GameDifficultyType options_difficulty_deserialize(const char* s) {
	if (strcmp(s, "easy") == 0) {
		return EASY;
	} else if (strcmp(s, "normal") == 0) {
		return NORMAL;
	} else if (strcmp(s, "hard") == 0) {
		return HARD;
	}
	return NORMAL;
}