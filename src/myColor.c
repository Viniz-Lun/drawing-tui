#include "headers/myColor.h"
#include <ncurses.h>

int get_color_index_from_rgb(RGB value, Color *colorArray, int maxDimColors){
	RGB toCompare;
	for( int i = 0; i < maxDimColors; i++){
		toCompare = colorArray[i].rgb;
		if( toCompare.r == value.r &&
				toCompare.g == value.g &&
				toCompare.b == value.b )
			return i;
	}
	return -1;
}

int get_color_index_from_num(short colorNum, Color *colorArray, int maxDimColors){
	for( int i = 0; i < maxDimColors; i++){
		if( colorArray[i].colorNum == colorNum )
			return i;
	}
	return -1;
}


int get_pair_index_from_rgb(RGB fg, RGB bg, Pair *pairArray, int maxDimPairs){
	RGB toCompare;
	for( int i = 0; i < 127; i++){
		if( pairArray[i].pairNum != 0 ){
			toCompare = pairArray[i].fg.rgb;
			if( toCompare.r == fg.r && toCompare.g == fg.g && toCompare.b == fg.b){
				toCompare = pairArray[i].bg.rgb;
				if( toCompare.r == bg.r && toCompare.g == bg.g && toCompare.b == bg.b){
					return pairArray[i].pairNum;
				}
			}
		}
	}
	return -1;
}

int get_pair_index_from_color_nums(short fg, short bg, Pair *pairArray, int arrLength){
	if( fg < 0 || bg < 0 ) return -1;
	for( int i = 0; i < LAST_PAIR - FIRST_PAIR; i++ ){
		if( pairArray[i].fg.colorNum == fg &&
				pairArray[i].bg.colorNum == bg ){
			return pairArray[i].pairNum;
		}
	}
	return -1;
}

Color make_new_color(RGB rgb, Color *colorCollection, int maxDimColors){
	short numOfColor;
	Color result;

	numOfColor = get_hole_in_short_sequence_array(colorCollection, maxDimColors, sizeof(Color));

	if( numOfColor == 1 ) numOfColor = FIRST_COLOR;

	result.colorNum = numOfColor;
	result.rgb = rgb;

	return result;
}

Pair make_new_color_pair(RGB fg, RGB bg, Color *customColors, int maxDimColors, Pair *customPairs, int maxDimPairs){
	short numOfPair;
	short fgColorIndex, bgColorIndex;
	Color color1, color2;
	Pair resultPair;

	fgColorIndex = get_color_index_from_rgb(fg, customColors, maxDimColors);
	bgColorIndex = get_color_index_from_rgb(bg, customColors, maxDimColors);

	if( fgColorIndex < 0 ){
		color1 = make_new_color(fg, customColors, maxDimColors);
		init_color(color1.colorNum, color1.rgb.r, color1.rgb.g, color1.rgb.b);
	}
	else color1 = customColors[fgColorIndex];

	if( bgColorIndex < 0 ){
		color2 = make_new_color(bg, customColors, maxDimColors);
		if( fgColorIndex < 0 ) color2.colorNum++;
		init_color(color2.colorNum, color2.rgb.r, color2.rgb.g, color2.rgb.b);
	}
	else color2 = customColors[bgColorIndex];
	
	if( color1.colorNum == 0 || color2.colorNum == 0 ) return (Pair){0};
	
	numOfPair = get_hole_in_short_sequence_array(customPairs, maxDimPairs, sizeof(Pair));
	if( numOfPair < 1 || numOfPair > LAST_PAIR ) return (Pair){0};
	
	resultPair.pairNum = numOfPair;
	resultPair.fg = color1;
	resultPair.bg = color2;

	init_pair(numOfPair, color1.colorNum, color2.colorNum);

	return resultPair;
}

void add_color(Color c, void* colorArray, int position, int maxDim){
	if( position >= maxDim ) return;
	Color* acc_color_array = (Color*)colorArray;
	acc_color_array[position] = c;
	return;
}

void add_pair(Pair p, void* pairArray, int position, int maxDim){
	if( position >= maxDim ) return;
	Pair* acc_pair_array = (Pair*)pairArray;
	acc_pair_array[position] = p;
	return;
}
