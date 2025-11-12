/* tuiWrapper.c */

#include "tuiWrapper.h"
#include <curses.h>

WinBorder defaultBorder = {
	1,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
};

void clear_Win(Win *win){
	wclear(win->ptr);
	win->borderSize = 0;
}

void remove_window(Win *win){
	wclear(win->ptr);
	wrefresh(win->ptr);
}

void delete_window(Win *win){
	remove_window(win);
	delwin(win->ptr);
}

Win *create_Win(int posy, int posx, int height, int width){
	WINDOW *windowPtr = newwin(height, width, posy, posx); 

	if ( windowPtr == NULL ) return NULL;

	Win *win = (Win*) malloc(sizeof(Win));
	win->cols = width;
	win->lines = height;
	win->ypos = posy;
	win->xpos = posx;
	win->borderSize = 0;
	win->ptr = windowPtr; 

	allWins = append( win, allWins );

	return win;
}

int move_Win( Win *win, int posy, int posx){
	if( mvwin( win->ptr, posy, posx) == OK ){
		// remove_window( win );
		win->xpos = posx;
		win->ypos = posy;
		touchwin( win->ptr );
		return OK;
	}
	return ERR;
}

void winborder(Win *window, WinBorder border){
	window->borderSize = border.size;
	wborder(window->ptr, border.left, border.right, border.top, border.bottom,
			border.topLeft_corner, border.topRight_corner, border.bottomLeft_corner, border.bottomRight_corner);
	for (int i= 1; i < border.size; i++){
		mvwhline(window->ptr, i, i, border.top, window->cols - i*2);
		mvwhline(window->ptr, window->lines - i, i, border.bottom, window->cols - i*2);
		mvwvline(window->ptr, i, i, border.left, window->lines - i*2);
		mvwvline(window->ptr, i, window->cols - i, border.left, window->lines - i*2);
		
		mvwaddch(window->ptr, i, i, border.topLeft_corner);
		mvwaddch(window->ptr, window->lines - i, i, border.bottomLeft_corner);
		mvwaddch(window->ptr, i, window->cols - i, border.topRight_corner);
		mvwaddch(window->ptr, window->lines - i, window->cols - i, border.bottomRight_corner);
	}
	return;
}

void winborder_offset(Win *window, WinBorder border, int offset){
	if(window->borderSize < offset + border.size) window->borderSize = offset + border.size;
	for (int i= offset; i < border.size + offset; i++){
		mvwhline(window->ptr, i, i, border.top, window->cols - i*2);
		mvwhline(window->ptr, window->lines - i, i, border.bottom, window->cols - i*2);
		mvwvline(window->ptr, i, i, border.left, window->lines - i*2);
		mvwvline(window->ptr, i, window->cols - i, border.left, window->lines - i*2);
		
		mvwaddch(window->ptr, i, i, border.topLeft_corner);
		mvwaddch(window->ptr, window->lines - i, i, border.bottomLeft_corner);
		mvwaddch(window->ptr, i, window->cols - i, border.topRight_corner);
		mvwaddch(window->ptr, window->lines - i, window->cols - i, border.bottomRight_corner);
	}
}

void clear_area(int posy, int posx, int height, int width){
	for(int i = 0; i < height; i++){
		mvhline(posy + i, posx, ' ', width);
	}
	refresh();
}

int get_xpos_for_string_size(int width, char *string, SIDE side, int offset){
	int x;
	switch(side){
		case SIDE_LEFT:
			x = offset;
			break;
		case SIDE_CENTER:
			x =  width/2 - strlen(string)/2 + offset;
			break;
		case SIDE_RIGHT:
			x = width- offset - 1 - strlen(string) + offset;
			break;
	}
	return x;
}

int get_xpos_for_string_window(Win window, char *string, SIDE side, int offset){
	if (side == SIDE_CENTER)
		return get_xpos_for_string_size(window.cols, string, side, offset);
	else return get_xpos_for_string_size(window.cols, string, side, window.borderSize + offset);
}

void read_input_echo(int y, int x, char *result, int max){
	int inputChar;
	char temp[MAXINPUT] = {0};
	int i;

	max = min(MAXINPUT, max);

	move(y, x);
	i = 0;
	inputChar = getch();
	while (! (inputChar == '\n' || inputChar == '\r' || inputChar == 10 || inputChar == KEY_F(1) || inputChar == 27)){
		if (inputChar <= 256 && i < max){
			temp[i++] = inputChar;
		}
		if (inputChar == KEY_BACKSPACE && i > 0){
			temp[--i] = '\0';
		}
		mvhline(y, x, ' ', max);
		move(y, x);
		printw("%s", temp);
		refresh();
		inputChar = getch();
	}
	
	temp[i] = '\0';
	
	if (inputChar == KEY_F(1)) {
		endwin();
		exit(0);
	}
	if (inputChar != 27) strncpy(result, temp, max);
	
	return;
}

