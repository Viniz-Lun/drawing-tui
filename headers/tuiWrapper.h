/* tuiWrapper.c */
#ifndef TUIWRAPPER_H
#define TUIWRAPPER_H

#include <ncurses.h>
#include "headers/win-wrapper.h"
#include "headers/list.h"

#ifndef MAXINPUT
	#define MAXINPUT 512
#endif

#define FIRST_COLOR 8
#define ESC 27
#define KEY_DELETE 330
#define KEY_TAB 9
#define ENTER 10


#define isMasc(A) A >= 'A' && A <= 'Z' 
#define isMinusc(A) A >= 'A' && A <= 'Z' 
#define isDigit(A) A >= '0' && A <= '9' 
#define isAlf(A) isMasc(A) || isMinusc(A)
#define min(A,B) (A < B)? A : B 
#define max(A,B) (A > B)? A : B

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
extern chtype emptyChar;

typedef enum {
	SIDE_LEFT,
	SIDE_CENTER,
	SIDE_RIGHT,
} SIDE;

void init_tui();

void set_Win_list( WinList list );

WinList get_Win_list();

void end_screen();

void delete_all_Win( WinList list );

void remove_window( Win *win );

void show_Win( Win *win );

void delete_Win( Win *win );

int move_Win( Win *win, int posy, int posx );

void clear_Win( Win *win );

Win *create_Win( int posy, int posx, int height, int width );

void border_Win(Win *window, WinBorder border);

void border_Win_offset(Win *window, WinBorder border, int offset);

void clear_area(int posy, int posx, int height, int width);

int get_xpos_for_string_size(int width, char *string, SIDE side, int offset);

int get_xpos_for_string_window(Win window, char *string, SIDE side, int offset);

int read_input_echo(Win *win, int y, int x, char *result, int max);

attr_t get_attr_off(attr_t current, attr_t toTurnOff);

attr_t get_attr_on(attr_t current, attr_t toTurnOn);

attr_t get_attr_with_color_pair(attr_t current, short pairNum);

#endif
