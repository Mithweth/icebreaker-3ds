// options.h
#pragma once


typedef enum { NORMAL, EASY, HARD } GameDifficultyType;


typedef struct
{
	GameDifficultyType difficulty;
    char theme[64];
} GameOptionsType;

extern GameOptionsType options;
