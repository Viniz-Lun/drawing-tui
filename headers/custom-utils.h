#include "RGB.h"

#define isMasc(A) A >= 'A' && A <= 'Z' 
#define isMinusc(A) A >= 'A' && A <= 'Z' 
#define isDigit(A) A >= '0' && A <= '9' 
#define isAlf(A) isMasc(A) || isMinusc(A)
#define min(A,B) (A < B)? A : B 
#define max(A,B) (A > B)? A : B
#define isTxt(A,LEN) A[LEN-1] == 't' && A[LEN-2] == 'x' && A[LEN-3] == 't' && A[LEN-4] == '.'

void RGB_to_hex(char *result, RGB rgb);

int hex_to_RGB(char* hexCode, RGB *rgb);

void pad_string_with_char_right(char* result, char c, int maxlen);

void pad_string_with_char_left(char* result, char c, int maxlen);

short get_hole_in_short_sequence_array(void* array, int dim, int step);
