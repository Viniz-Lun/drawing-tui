#ifndef COLOR_H
#define COLOR_H

#include "RGB.h"
#include <ncurses.h>
#include "custom-utils.h"

#define FIRST_COLOR 8
#define LAST_COLOR 253
#define FIRST_PAIR 1
#define LAST_PAIR 127

typedef struct color_t{
	short colorNum;
	RGB rgb;
} Color;

typedef struct pair_t{
	short pairNum;
	Color fg;
	Color bg;
} Pair;


int get_color_index_from_rgb(RGB value, Color *colorArray, int maxDimColors);

int get_color_index_from_num(short colorNum, Color *colorArray, int maxDimColors);

int get_pair_index_from_color_nums(short fg, short bg, Pair* pairArray, int arrLength);

//TODO
int get_pair_index_from_rgb(RGB fg, RGB bg, Pair* colorPairs, int maxDimPairs);

Color make_new_color(RGB rgb, Color* colorCollection, int maxDimColors);

//TODO
Pair make_new_color_pair(RGB fg, RGB bg, Color* customColors, int maxDimColors, Pair* customPairs, int maxDimPairs);

void add_color(Color c, void* colorArray, int position, int maxDim);

void add_pair(Pair p, void* pairArray, int position, int maxDim);

#endif
