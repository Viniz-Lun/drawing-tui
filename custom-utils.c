#include "headers/custom-utils.h"

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
	float f;
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
