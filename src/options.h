// options.h
#pragma once


typedef enum { DIFFICULTY_NORMAL, DIFFICULTY_EASY, DIFFICULTY_HARD } GameDifficultyType;


typedef struct
{
	GameDifficultyType difficulty;
    char theme[64];
} GameOptionsType;

extern GameOptionsType options;
