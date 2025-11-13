/* tuiWrapper.c */
#ifndef TUIWRAPPER_H
#define TUIWRAPPER_H

#include <curses.h>
#include <string.h>
#include "list.h"

#ifndef MAXINPUT
	#define MAXINPUT 512
#endif


#define isMasc(A) A >= 'A' && A <= 'Z' 
#define isMinusc(A) A >= 'A' && A <= 'Z' 
#define isDigit(A) A >= '0' && A <= '9' 
#define isAlf(A) isMasc(A) || isMinusc(A)
#define min(A,B) (A < B)? A : B 
#define max(A,B) (A > B)? A : B
#define isCurse(A,LEN) A[LEN-1] == 'e' && A[LEN-2] == 's' && A[LEN-3] == 'r' && A[LEN-4] == 'u' && A[LEN-5] == 'c' && A[LEN-6] == '.'

typedef struct {
	int size;
	int left;
	int right;
	int top;
	int bottom;
	int topLeft_corner;
	int topRight_corner;
	int bottomRight_corner;
	int bottomLeft_corner;
} WinBorder;

extern Win baseScr;
extern WinBorder defaultBorder;
extern WinList allWins;

struct Win{
	WINDOW *ptr;
	int lines;
	int cols;
	int xpos;
	int ypos;
	int borderSize;
	int hidden;
};

//typedef struct {
//	int mode;
//	char toPrint[20];
//	int BITMASK;
//	int dx;
//	int dy;
//} state;

typedef enum {
	SIDE_LEFT,
	SIDE_CENTER,
	SIDE_RIGHT,
} SIDE;

//typedef enum{
//	TOP_LEFT,
//	TOP_CENTER,
//	TOP_RIGHT,
//	MIDDLE_LEFT,
//	MIDDLE_CENTER,
//	MIDDLE_RIGHT,
//	BOTTOM_LEFT,
//	BOTTOM_CENTER,
//	BOTTOM_RIGHT,
//} POSITION;

void end_screen();

void remove_window(Win *win);

void delete_Win(Win *win);

int move_Win( Win *win, int posy, int posx);

Win *create_Win(int posy, int posx, int height, int width);

void winborder(Win *window, WinBorder border);

void winborder_offset(Win *window, WinBorder border, int offset);

void clear_area(int posy, int posx, int height, int width);

int get_xpos_for_string_size(int width, char *string, SIDE side, int offset);

int get_xpos_for_string_window(Win window, char *string, SIDE side, int offset);

void wread_input_echo(Win *win, int y, int x, char *result, int max);

#define read_input_echo(y, x, result, max); wread_input_echo(&baseScr, y, x, result, max);
#endif
