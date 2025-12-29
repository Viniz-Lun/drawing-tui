#ifndef RGB_H
	#define RGB_H
	#define round_up_division_int( NUM, DENUM ) ((int)((NUM) / (DENUM)) + ((NUM) % (DENUM) > 0))
	typedef struct{
		short r;
		short g;
		short b;
	} RGB;
#endif
