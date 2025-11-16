/* tuiWrapper.c */

#include "tuiWrapper.h"
#include "list.h"
#include <curses.h>

static WinList allWins;

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

Win baseScr = {
	0,
	0,
	0,
	0,
	0,
	0,
	0,
};

WinList get_Win_list();
chtype emptyChar = {' '};

void set_Win_list(WinList list){
	allWins = list;
}

WinList get_Win_list(){
	return allWins;
}

void end_screen(){
	endwin();
	WinList tempList = allWins;
	while( ! is_empty(tempList) ){
		if( head(tempList) != &baseScr ) free( head(tempList) );
		tempList = tail(tempList);
	}
	free_list(allWins);
}

void clear_Win(Win *win){
	wclear(win->ptr);
	win->borderSize = 0;
}

int posy_first_intersect(Win* Awin, Win* Bwin){
	//int miny;
	if( Awin->ypos >= Bwin->ypos && Awin->ypos <= (Bwin->ypos + Bwin->lines) ){
		//miny = Awin->ypos;
		return Awin->ypos;
	}
	else{
		if( Bwin->ypos >= Awin->ypos && Bwin->ypos <= (Awin->ypos + Awin->lines) ) 
			//miny = Bwin->ypos;
			return Bwin->ypos;
		else return -1;
	}

	//if( Awin->xpos >= Bwin->xpos && Awin->xpos <= (Bwin->xpos + Bwin->cols) ||
	//		Bwin->xpos >= Awin->xpos && Bwin->xpos <= (Awin->xpos + Awin->cols) ){
	//	return miny;
	//}
	//return -1;
}

int num_lines_intersect(Win* Awin, Win* Bwin){
	int firstPosy, lastPosy;
	if( (firstPosy = posy_first_intersect(Awin, Bwin)) >= 0){
		lastPosy = min((Awin->ypos + Awin->lines), (Bwin->ypos + Bwin->lines));
		return lastPosy - firstPosy;
	}
	else return -1;
}

void remove_window(Win *win){
	int intersectPoint, numLines;
	clear_area(win->ypos, win->xpos, win->lines, win->cols);
	win->hidden = 1;
	WinList tempList = allWins;
	Win *currentWin;
	while( ! is_empty(tempList) ){
		currentWin = head(tempList);
		if( ! currentWin->hidden ){
			intersectPoint = posy_first_intersect(win, currentWin);
			if( intersectPoint >= 0 ){
				touchline( currentWin->ptr,
						intersectPoint - currentWin->ypos,
						num_lines_intersect(win, currentWin));
				wrefresh(currentWin->ptr);
			}
		}
		tempList = tail(tempList);
	}
}

void show_Win( Win* win ){
	win->hidden = 0;
	touchwin( win->ptr );
	wrefresh( win->ptr );
}

void delete_Win(Win *win){
	remove_window(win);
	delwin(win->ptr);
	allWins = remove_element(win, allWins);
	free( win );
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
	win->hidden = 0;

	allWins = append( win, allWins );

	return win;
}

int move_Win( Win *win, int posy, int posx){
	if( mvwin( win->ptr, posy, posx) == OK ){
		remove_window( win );
		win->hidden = 0;
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
			x = width - offset - 1 - strlen(string);
			break;
	}
	return x;
}

int get_xpos_for_string_window(Win window, char *string, SIDE side, int offset){
	if (side == SIDE_CENTER)
		return get_xpos_for_string_size(window.cols, string, side, offset);
	else return get_xpos_for_string_size(window.cols, string, side, window.borderSize + offset);
}

void wread_input_echo(Win* win, int y, int x, char *result, int max){
	int inputChar;
	char temp[MAXINPUT] = {0};
	int i;

	curs_set(1);

	max = min(MAXINPUT, max);

	wmove(win->ptr, y, x);
	i = 0;
	inputChar = wgetch(win->ptr);
	while (! (inputChar == '\n' || inputChar == '\r' || inputChar == 10 || inputChar == KEY_F(1) || inputChar == 27)){
		if (inputChar <= 256 && i < max){
			temp[i++] = inputChar;
		}
		if (inputChar == KEY_BACKSPACE && i > 0){
			temp[--i] = '\0';
		}
		mvwhline(win->ptr, y, x, ' ', max);
		wmove(win->ptr, y, x);
		wprintw(win->ptr, "%s", temp);
		wrefresh(win->ptr);
		inputChar = wgetch(win->ptr);
	}
	
	temp[i] = '\0';
	
	if (inputChar != 27 && inputChar != KEY_F(1)) strncpy(result, temp, max);
	
	return;

}



