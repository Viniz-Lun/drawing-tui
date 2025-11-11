/* triangle.c */
#include <curses.h>
#include <stdbool.h>
#include <stdio.h>
#include <fcntl.h>
#include <sys/types.h>
#include <unistd.h>

#include "list.h"
#include "tuiWrapper.h"

#define ESC 27

typedef enum {
	INSERT,
	DELETE,
	NORMAL,
	SELECT,
	VISUAL,
} MODE;

typedef struct {
	char toPrint[MAXINPUT];
	MODE mode;
	char chMask;
	u_int focus;
} State;

State currentState;

chtype emptyChar[2] = {' ', 0};
Win baseScr;

char *options[] = {
	"Set String",
	"Set color pair",
	"Toggle reverse",
	"Toggle automatic insert",
	"Load Image from file",
	"Save Image",
	"Quit without Saving",
};

int numOptions = 7;

char toPrint[MAXINPUT] = {0};
int skip, cancel;
Win baseScr;
WinList allWins = NULL;

void init(){
	initscr();
	cbreak();
	noecho();
	set_escdelay(50);
	
	allWins = append( &baseScr, allWins );
	
	defaultBorder.size = 1;
	defaultBorder.bottom = ACS_HLINE;
	defaultBorder.top = ACS_HLINE;
	defaultBorder.right = ACS_VLINE;
	defaultBorder.left = ACS_VLINE;
	defaultBorder.topLeft_corner = ACS_ULCORNER;
	defaultBorder.topRight_corner = ACS_URCORNER;
	defaultBorder.bottomLeft_corner = ACS_LLCORNER;
	defaultBorder.bottomRight_corner = ACS_LRCORNER;

	baseScr.ptr = stdscr;
	baseScr.cols = COLS;
	baseScr.lines = LINES;
	baseScr.borderSize = 0;
	baseScr.xpos = 0;
	baseScr.ypos = 0;
}

MODE get_mode( MODE prev_mode, MODE curr_mode, int key ){
	switch(key){
		case ESC:
			return NORMAL;
		case 10:
			return (curr_mode == INSERT)? NORMAL:INSERT;
		case KEY_BACKSPACE:
			return (curr_mode == DELETE)? NORMAL:DELETE;
		case KEY_CTAB:
			return (curr_mode == SELECT)? NORMAL:SELECT;
		case KEY_CANCEL:
			return (curr_mode == VISUAL)? NORMAL:VISUAL;
		default:
			return NORMAL;
	}
}

void update_hud(){
	char string[MAXINPUT];

	mvhline(LINES - 1, 1, ACS_HLINE, COLS - 2);

	strncpy(string, " Press F1 to quit ", MAXINPUT);
	mvaddstr(LINES - 1, get_xpos_for_string_window(baseScr, string, SIDE_LEFT, 1), string);

	sprintf(string, " Brush: '%s' ", toPrint);
	mvaddstr(LINES - 1, get_xpos_for_string_size(COLS, string, SIDE_CENTER, 1), string);

	sprintf(string, " %s ", ( currentState.mode == NORMAL )? "NORMAL": (currentState.mode == DELETE)? "DELETE": "SELECT");
	//sprintf(string, " SKIP: [%c] DELETE: [%c] ", (skip)? 'x': ' ', (cancel)? 'x':' ' );
	mvaddstr(LINES - 1, get_xpos_for_string_size(COLS, string, SIDE_RIGHT, 1), string);
	refresh();
}

// void posaddstr(Win window, char *string, POSITION pos){}

void setup_menu_popup(Win *window, char *title, SIDE title_centering, char **options, int numOptions, SIDE text_centering){
	int y, x;
	window->borderSize = 1;
	
	if(options != NULL){
		numOptions = (numOptions > window->lines - window->borderSize*2)? window->lines - window->borderSize*2 : numOptions;
		for (int i = 0; i < numOptions; i++){
			y = i + window->borderSize;
			x = get_xpos_for_string_window(*window, options[i], text_centering, 0);

			mvwhline(window->ptr, y, 0, ' ', window->cols);
			mvwaddnstr(window->ptr, y, x, options[i], window->cols - window->borderSize * 2);
		}
	}
	box(window->ptr, 0,0);

	x = get_xpos_for_string_window(*window, title, title_centering, 0);
	mvwaddstr(window->ptr, 0, x, title);
}


void print_input_menu(int posy, int posx, int width, WinBorder border, char* printPrompt, char* optional_message){
	int i, height, len, inputChar;
	char prompt[200];
	strncpy(prompt, printPrompt, 200);
	
	curs_set(1);

	height = 3; 
	
	mvaddch(posy, posx, border.topLeft_corner);
	mvaddch(posy + height-1, posx, border.bottomLeft_corner);
	mvaddch(posy, posx + width-1, border.topRight_corner);
	mvaddch(posy + height-1, posx + width -1, border.bottomRight_corner);
	
	mvhline(posy, posx+1, border.top, width-2);
	mvhline(posy + height-1 , posx+1, border.bottom, width-2);
	
	mvvline(posy+1, posx, border.left, height-2);
	mvvline(posy+1, posx + width-1, border.right, height-2);
	
	len = strlen(printPrompt);
	
	if (len > width) prompt[width] = '\0';
	mvaddstr(posy, posx + (width/2 - len/2), prompt);

	if(optional_message != NULL){
		len = strlen(optional_message);
			if (len > width) optional_message[width] = '\0';
		mvaddstr(posy + height-1, posx + (width/2 - len/2), optional_message);
	}
	refresh();
	
	return;
}

int save_drawing_to_file(Win *window, char *file_name){
	
	int fd;
	chtype *buffer;
	char stringExitMsg[32];
	
	buffer = (chtype*) malloc(window->cols * sizeof(chtype) +1);
	
	fd = open(file_name, O_WRONLY | O_CREAT, 0644);
	if (fd < 0){
		strncpy(stringExitMsg, " Error opening/creating the file ", 32);
		perror(stringExitMsg);
		mvaddstr(0, 0, stringExitMsg);
		refresh();
		free(buffer);
		return 1;
	}
	for(int i = 0; i < window->lines; i++){
		mvwinchstr(window->ptr, i, 0, buffer);
		write(fd, buffer, sizeof(chtype) * window->cols);
		*buffer = '\n';
		write(fd, buffer, sizeof(chtype));
	}
	free(buffer);
	close(fd);
	sprintf(stringExitMsg, " Done saving file, size: %lu ", window->cols * window->lines * sizeof(char) + sizeof(char) * window->lines );
	mvaddstr(0, 0, stringExitMsg);
	refresh();
	return 0;
}

int load_image_from_file(Win *window, char *file_name){
	int fd;
	int i, o, nread, len;
	char stringExitMsg[32];

	fd = open(file_name, O_RDONLY);
	if (fd < 0){
		mvhline(0, 0, COLS, ' ');
		strncpy(stringExitMsg, " Error opening the file ", 32);
		mvaddstr(0, 0, stringExitMsg);
		refresh();
		return 1;
	}

	len = strlen(file_name);
	if (isCurse(file_name, len)){
		chtype *chtypeBuffer;
		chtypeBuffer = (chtype*) malloc(window->cols * sizeof(chtype) +1);
		
		for(i = 0; i < window->lines; i++){
			for(o = 0; o < window->cols + 1; o++){
				nread = read(fd,&chtypeBuffer[o], sizeof(chtype));
				if ((chtypeBuffer[o] & A_CHARTEXT) == '\n' || nread <= 0){
					break;
				}
			}
			if (nread >= 0) {
				chtypeBuffer[o] = 0;
				mvwaddchnstr(window->ptr, i, 0, chtypeBuffer, o);
			}
			if (nread <= 0) break;
		}
		
		free(chtypeBuffer);
	}
	else{
		char *charBuffer;
		charBuffer = (char*) malloc(window->cols * sizeof(char) +1);
		
		for(i = 0; i < window->lines; i++){
			for(o = 0; o < window->cols + 1; o++){
				nread = read(fd, &charBuffer[o], sizeof(char));
				if (charBuffer[o] == '\n' || nread <= 0){
					break;
				}
			}
			if (nread >= 0) {
				charBuffer[o] = 0;
				mvwaddnstr(window->ptr, i, 0, charBuffer, o);
			}
			if (nread <= 0) break;
		}
		
		free(charBuffer);
	}

	close(fd);

	if (nread >= 0) {
		sprintf(stringExitMsg, " Done loading file: %s ", file_name);
		touchwin(window->ptr);
	}
	else {
		sprintf(stringExitMsg, "Error reading from file");
	}

	mvhline(0, 0, COLS, ' ');
	mvaddstr(0, 0, stringExitMsg);
	refresh();
	return 0;
}


void highlight_menu_line(Win* window, int lineNum, bool highlight){
	chtype* buffer;
	int x, y, size;

	buffer = (chtype*) malloc(window->cols * sizeof(chtype) +1);

	curs_set(0);

	x = window->borderSize;
	y = lineNum + window->borderSize;
	size = window->cols - window->borderSize * 2;

	mvwinchnstr(window->ptr, y, x, buffer, size);
	if(highlight){
		for(int i = 0; i < size; i++){
			buffer[i] = buffer[i] | A_REVERSE;
		}
	}
	else{
		for(int i = 0; i < size; i++){
			buffer[i] = buffer[i] & (~A_REVERSE);
		}
	}

	mvwaddchnstr(window->ptr, y, x, buffer, size);

	free(buffer);
	wrefresh(window->ptr);
	return;
}

int handle_enter(Win *window,int optNum){
	int fd;
	char file_name[MAXINPUT] = {0};
	int posy, posx;
	int maxChars;

	posy = LINES/5 * 4;
	posx = COLS/2 - COLS/8;
	maxChars = min(MAXINPUT - 1, COLS/4);

	switch(optNum){
		case 0:
			print_input_menu(posy, posx, maxChars + 2, defaultBorder,
					"Write string to use:", NULL); 
			read_input_echo(posy + 1, posx + 1, toPrint, maxChars); 
			clear_area(posy, posx, 3, maxChars + 2);
			
			curs_set(0);
			update_hud();
			break;
		case 2:
			if (getattrs(window->ptr) & A_REVERSE) wattroff(window->ptr, A_REVERSE);
			else wattron(window->ptr, A_REVERSE);
			break;
		case 4:
			print_input_menu(posy, posx, maxChars + 2, defaultBorder,
					"Insert file name (current directory):", "Esc to cancel");
			read_input_echo(posy + 1, posx + 1, file_name, maxChars); 
			clear_area(posy, posx, 3, maxChars + 2);
			curs_set(0);
			
			if(file_name[0] != 0){
				load_image_from_file(window, file_name);
				return 1;
			}
			break;
		case 5:
			print_input_menu(posy, posx, maxChars + 2, defaultBorder,
					"Insert file name (current directory):", "Esc to cancel");
			read_input_echo(posy + 1, posx + 1, file_name, maxChars); 
			clear_area(posy, posx, 3, maxChars + 2);
			curs_set(0);
			
			if(file_name[0] != 0){
				save_drawing_to_file(window, file_name);
			}
			break;
		case 6:
			free_list(allWins);
			endwin();
			exit(0);
			break;
	}
	return 0;
}

int main(int argc, char **argv){
	
	int startx, starty, dx, dy;
	int inp;
	int drawFocus;
	int highlight;
	Win *popupWin, *drawWin;

	/* initialize curses */

	init();

	//initialize drawWin
	//drawWin.cols = COLS - 2;
	//drawWin.lines = LINES - 2;
	//drawWin.xpos = 1;
	//drawWin.ypos = 1;
	//drawWin.borderSize = 0;
	//drawWin.ptr = newwin(drawWin.lines, drawWin.cols, drawWin.xpos, drawWin.ypos);
	drawWin = create_Win(1, 1, LINES - 2, COLS - 2);

	if ( drawWin != NULL ) allWins = append(drawWin, allWins);

	//initialize popupWin
	//popupWin.cols = 40;
	//popupWin.lines = 10;
	//popupWin.xpos = COLS/2 - (popupWin.cols/2);
	//popupWin.ypos = LINES/2 - (popupWin.lines/2);
	//popupWin.borderSize = 1;
	//popupWin.ptr = newwin(popupWin.lines, popupWin.cols, popupWin.ypos, popupWin.xpos);
	int posy, posx;
	posy = LINES/2 - 5;
	posx = COLS/2 - 20;
	popupWin = create_Win( posy, posx, 10, 40 );

	if ( popupWin != NULL ) allWins = append(popupWin, allWins);

	
	//keypad(stdscr, 1);
	keypad(stdscr, 1);
	keypad(drawWin->ptr, 1);
	keypad(popupWin->ptr, 1);

	starty = drawWin->lines / 2;
	startx = drawWin->cols / 2;
	dy = starty;
	dx = startx;

	drawFocus = 1;
	skip = 1;
	cancel = 0;
	highlight = 0;

	toPrint[0] = ' '; 

	box(stdscr,0,0);
	update_hud();
	
	wmove(drawWin->ptr, starty, startx);
	setup_menu_popup(popupWin, "| MENU |", SIDE_LEFT, options, numOptions, SIDE_CENTER);

	
	while( (inp = (drawFocus)? wgetch(drawWin->ptr) : wgetch(popupWin->ptr)) && inp != KEY_F(1) ){
		if(drawFocus){
			switch(inp){
				case KEY_F(2):
					touchwin(popupWin->ptr);
					highlight_menu_line(popupWin, highlight, false);
					drawFocus = 0;
					highlight_menu_line(popupWin, highlight, true);
					break;
				case KEY_LEFT:
					if (dx > 0) --dx;
					break;
				case KEY_RIGHT:
					if (dx < drawWin->cols - 1) ++dx;
					break;
				case KEY_UP:
					if (dy > 0) --dy;
					break;
				case KEY_DOWN:
					if (dy < drawWin->lines - 1) ++dy;
					break;
				case 27:
					skip = !skip;
					break;
				case KEY_BACKSPACE:
					cancel = !cancel;
					break;
				default :
					if(inp >= 32 && inp <=136){
						cancel = 0;
						toPrint[0] = inp;
						toPrint[1] = 0;
					}
			}
			update_hud();
			if( (inp == KEY_ENTER) || (drawFocus && !skip) ){
				if(cancel){
					mvwaddchstr(drawWin->ptr, dy, dx , emptyChar); 
				}
				else mvwaddstr(drawWin->ptr, dy, dx , toPrint);
				wrefresh(drawWin->ptr);
			}
			wmove(drawWin->ptr, dy, dx);
		}
		else{
			//MENU navigation
			switch(inp){
				case KEY_UP:
					highlight_menu_line(popupWin, highlight,false);
					if(highlight == 0) highlight = numOptions - 1;
					else highlight--;
					highlight_menu_line(popupWin, highlight,true);
					break;
				case KEY_DOWN:
					highlight_menu_line(popupWin, highlight,false);
					if(highlight == numOptions - 1) highlight = 0;
					else highlight++;
					highlight_menu_line(popupWin, highlight,true);
					break;
				case 10:
					if (handle_enter(drawWin, highlight) == 0) break;
				case 27: 
					drawFocus = 1;
					touchwin(drawWin->ptr);
					wrefresh(drawWin->ptr);
					wmove(drawWin->ptr, dy, dx);
					curs_set(1);
					break;
				default :
					break;
			}
		}
	} 
	free_list(allWins);
	endwin();
	
	return 0;
}
