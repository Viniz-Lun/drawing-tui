#include "custom-utils.h"

#include <string.h>
#include <stdlib.h>
#include <stdio.h>

void RGB_to_hex(char *result, RGB rgb){
	int value;
	int tempValue;
	value = 0;

	tempValue = rgb.r * 255;
	value += round_up_division_int(tempValue, 1000) << 16;
	tempValue = rgb.g * 255;
	value += round_up_division_int(tempValue, 1000) << 8;
	tempValue = rgb.b * 255;
	value += round_up_division_int(tempValue, 1000);

	snprintf(result, 7, "%X", value);
	pad_string_with_char_left(result, '0', 6);
}

int hex_to_RGB(char* hexCode, RGB *rgb){
	int value;
	for( int i = 0; i < 6; i++){
		if(! ( isDigit(hexCode[i]) ||
				(hexCode[i] >= 'a' && hexCode[i] <= 'f') ||
				(hexCode[i] >= 'A' && hexCode[i] <= 'F') 
				) ) return 0;
	}
	value = strtol(hexCode, NULL, 16);
	rgb->b = (value & 0xff);
	rgb->g = ((value >> 8) & 0xff); 
	rgb->r = (value >> 16);

	rgb->r = ((rgb->r * 1000) / 255);
	rgb->g = ((rgb->g * 1000) / 255);
	rgb->b = ((rgb->b * 1000) / 255);

	return 1;
}

void pad_string_with_char_right(char* result, char c, int maxlen){
	int i;

	i = strlen(result);

	if ( i < maxlen ){
		for(; i < maxlen ; i++ ){
			result[i] = c;
		}
		result[i] = '\0';
	}
	else return;
}

void pad_string_with_char_left(char* result, char c, int maxlen){
	int i;
	int len;
	int numOfC;

	len = strlen(result);
	numOfC = maxlen - len;

	if( numOfC <= 0) return;
	
	result[maxlen] = '\0';
	for( i = 0; i < len; i++ ){
		result[ (maxlen-1) - i ] = result[ (len-1) - i];
		result[ (len-1) - i ] = c;
	}
	//i = len
	for(; i < numOfC; i++ ){
		result[i] = c;
	}
	return;
}

// Finds the first "hole" in a sequence of unsorted short numbers, assuming the first
// value is the starting point, with the variable sizeOfContainer being used
// in case void *array points to a different structure array than short. However, the
// number value compared will still be of short type (2 bytes in x64).
short get_hole_in_short_sequence_array(void *array, int dim, int sizeOfContainer){
	unsigned long bitmask[5];
	unsigned long step;
	unsigned int i, j;

	short *shortPtr = (short*)array;
	short firstValue = *shortPtr;

	step = sizeOfContainer / sizeof(short);

	bitmask[0] = 1;
	for( i = 1; i < dim && i < (sizeof(*bitmask) * 8 * 5); i++){ 
		bitmask[i/64] |= (long)1 << ( (shortPtr[i * step] - firstValue) % 64 );
	}

	for( j = 1; j < i; j++ ){
		if( ( bitmask[j/64] & ((long)1 << (j % 64)) ) == 0 ) 
			return j + firstValue;
	}

	return dim + firstValue;
}
