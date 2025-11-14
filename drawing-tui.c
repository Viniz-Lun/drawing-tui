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
#define ENTER 10
#define COLOR_PREVIEW 255

typedef enum {
	INSERT,
	DELETE,
	NORMAL,
	SELECT,
	VISUAL,
	STICKY,
} MODE;

typedef struct{
	u_int r;
	u_int g;
	u_int b;
} RGB;

typedef struct {
	char toPrint[MAXINPUT];
	MODE mode;
	u_int chMask;
	u_int focus;
} State;

State currentState;

chtype emptyChar[2] = {' ', 0};

char *options[] = {
	"Set Brush string",
	"Set color pair",
	"Toggle reverse",
	"Toggle Sticky mode",
	"Load Image from file",
	"Save Image",
	"Help",
	"Quit without Saving",
};

int numOptions = 8;
int skip, cancel;
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
	
	currentState.toPrint[0] = ' '; 
	currentState.toPrint[1] = 0;
	currentState.mode = NORMAL;
	currentState.chMask = 0;
	currentState.focus = 1;
}

MODE get_mode(MODE curr_mode, int key ){
	if ( curr_mode == STICKY ) return STICKY;
	switch(key){
		case ESC:
			return NORMAL;
		case ENTER:
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

void show_message_top_left(char* message){
	mvaddch(0, 0, ACS_ULCORNER);
	mvhline(0, 1, ACS_HLINE, COLS - 2);
	mvaddstr(0, 2, message);
	refresh();
}

void get_mode_nstring(char* dest, MODE mode, int maxlen){
	switch(mode){
		case NORMAL:
			strncpy(dest, "NORMAL",maxlen);
			break;
		case INSERT:
			strncpy(dest, "INSERT",maxlen);
			break;
		case SELECT:
			strncpy(dest, "SELECT",maxlen);
			break;
		case DELETE:
			strncpy(dest, "DELETE",maxlen);
			break;
		case VISUAL:
			strncpy(dest, "VISUAL",maxlen);
			break;
		case STICKY:
			strncpy(dest, "STICKY", maxlen);
			break;
		default:
			strncpy(dest, "NORMAL",maxlen);
			break;
	}
}

void update_hud(){
	char string[MAXINPUT];
	char modeString[8];

	mvhline(LINES - 1, 1, ACS_HLINE, COLS - 2);

	strncpy(string, " PALETTE: $  ", MAXINPUT);
	mvaddstr(LINES - 1, get_xpos_for_string_window(baseScr, string, SIDE_LEFT, 1), string);
	mvaddch( LINES - 1, get_xpos_for_string_size(LINES, "", SIDE_LEFT, strlen(string) - 1), ' ' | currentState.chMask );

	sprintf(string, " Brush: '%s' ", currentState.toPrint);
	mvaddstr(LINES - 1, get_xpos_for_string_size(COLS, string, SIDE_CENTER, 0), string);

	get_mode_nstring(modeString, currentState.mode, 7);
	sprintf(string, "[ %s ]", modeString);
	//sprintf(string, " SKIP: [%c] DELETE: [%c] ", (skip)? 'x': ' ', (cancel)? 'x':' ' );
	mvaddstr(LINES - 1, get_xpos_for_string_size(COLS, string, SIDE_RIGHT, 1), string);
	refresh();
}

// void posaddstr(Win window, char *string, POSITION pos){}

void setup_menu_popup(Win *window, char *title, SIDE title_centering, char **options, int numOptions, SIDE text_centering){
	int y, x;
	winborder(window, defaultBorder);
	
	if(options != NULL){
		numOptions = (numOptions > window->lines - window->borderSize*2)? window->lines - window->borderSize*2 : numOptions;
		for (int i = 0; i < numOptions; i++){
			y = i + window->borderSize;
			x = get_xpos_for_string_window(*window, options[i], text_centering, 0);

			mvwhline( window->ptr, y, window->borderSize, ' ', window->cols - (window->borderSize * 2) );
			mvwaddnstr( window->ptr, y, x, options[i], window->cols - (window->borderSize * 2) );
		}
	}

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
		show_message_top_left(stringExitMsg);
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
	show_message_top_left(stringExitMsg);
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
		show_message_top_left(stringExitMsg);
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
	show_message_top_left(stringExitMsg);
	refresh();
	return 0;
}

void print_help_screen(){
	Win *helpWin = create_Win(10, 10, 30, 60);
	curs_set(0);

	mvwaddstr(helpWin->ptr, 1, 1, 
	"So this is the help menu, to start drawing you can\n\
 press <Esc> to exit the menu and enter the \n\
 drawing screen.\n\
 MODES:\n\
 There are currently 4 modes implemented, which are:\n\
 NORMAL\n\
 - Key: <Esc>\n\
 - The mode used to move around the screen without\n\
 modifying the drawing.\n\
 INSERT:\n\
 - Key: <Enter>\n\
 - The mode to actually draw on the screen, \n\
 whenever your cursor moves it places the content of\n\
 the brush at the cursor position in this mode.\n\
 DELETE:\n\
 - Key: <BackSpace>\n\
 - The mode to put an empty space wherever your\n\
 cursor moves.\n\
 STICKY:\n\
 - Key: menu option\n\
 - This mode is different from the others, instead\n\
 of switching modes, each key press is it's own\n\
 action, without switching to the respective mode,\n\
 acting like a constant NORMAL mode.\n\
 For example, whenever you press <Enter> it places\n\
 the content of the brush at the position of the cursor\n\
 while staying in the STICKY mode, same with <BackSpace>.\n\
 ---Press any key to exit Help menu---\
");

	setup_menu_popup(helpWin, "| Help menu |", SIDE_CENTER, NULL, 0, 0);

	wgetch(helpWin->ptr);
	delete_Win(helpWin);
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

RGB hex_parse(char* hexCode){
	return (RGB){ 900, 100, 100 };
}

int option_picker(Win* win, int numOptions, int* hover){
	int highlight = 0;
	int input = 0;

	if( ! is_keypad( win->ptr ) ) keypad( win->ptr, true );

	curs_set(0);
	
	if( hover != NULL ) {
		highlight = *hover;
		if( highlight < 0 ) highlight = highlight * -1;
		if( highlight >= numOptions ) highlight = highlight % (numOptions - 1);
	}

	highlight_menu_line(win, highlight,true);
	while( (input = wgetch(win->ptr)) ){
		switch(input){
			case KEY_UP:
				highlight_menu_line(win, highlight,false);
				if(highlight == 0) highlight = numOptions - 1;
				else highlight--;
				highlight_menu_line(win, highlight,true);
				break;
			case KEY_DOWN:
				highlight_menu_line(win, highlight,false);
				if(highlight == numOptions - 1) highlight = 0;
				else highlight++;
				highlight_menu_line(win, highlight,true);
				break;
			case ENTER:
				if( hover !=NULL ) *hover = highlight;
				return highlight;
			case ESC: 
				if( hover !=NULL ) *hover = highlight;
				return -1;
				break;
			case KEY_F(1):
				return -2;
				break;
			default :
				break;
		}
	}
	return -10;
}

void fill_string_with_char(char* result, char c, int maxlen){
	int i;
	for( i = 0; i < maxlen && result[i] != '\0'; i++ );
	if ( i < maxlen ){
		for(; i < maxlen ; i++ ){
			result[i] = '0';
		}
		result[i] = '\0';
	}
	else return;

}

int change_color_popup(Win* drawWin){
	Win* color_win;
	char hex_code[7];
	RGB rgb1;
	RGB rgb2;
	int optionSelected = 0;
	int posLetterHex;
	int posBackHex;
	int posYHex;
	char *color_options[] ={
		"Choose letter color",
		"Choose background color",
	};

	color_win = create_Win(10,10,10,30);
	setup_menu_popup(color_win, "| Color picker |", SIDE_CENTER, color_options, 2, SIDE_LEFT);
	wrefresh(color_win->ptr);
	curs_set(1);
	posYHex = color_win->lines - color_win->borderSize - 1;
	posLetterHex = color_win->borderSize + 2;
	posBackHex = color_win->cols - color_win->borderSize - 7;

	mvwaddstr(color_win->ptr, posYHex, posLetterHex - 1, "#000000");
	mvwaddstr(color_win->ptr, posYHex, posBackHex - 1, "#000000");

	hex_code[0] = '\0';

	while( (optionSelected = option_picker(color_win, 2, &optionSelected)) >= 0 ){
		curs_set(1);
		switch(optionSelected){
			case 0:
				wread_input_echo(color_win, posYHex, posLetterHex, hex_code, 6);
				fill_string_with_char(hex_code, '0', 6);
				mvwaddstr(color_win->ptr, posYHex, posLetterHex, hex_code);
				break;
			case 1:
				wread_input_echo(color_win, posYHex, posBackHex, hex_code, 6);
				fill_string_with_char(hex_code, '0', 6);
				mvwaddstr(color_win->ptr, posYHex, posBackHex, hex_code);
				break;
			default:
				break;
		}
		hex_code[0] = '\0';
	}
	if( optionSelected == -2 ){
		end_screen();
		exit(0);
	}

	init_color(COLOR_RED, rgb1.r, rgb1.g, rgb1.b);
	init_color(COLOR_BLUE, rgb2.r, rgb2.g, rgb2.b);
	init_pair( 1, COLOR_RED, COLOR_BLUE);
	
	wattron(drawWin->ptr, COLOR_PAIR(1));

	delete_Win(color_win);
	return 0;
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
			read_input_echo(posy + 1, posx + 1, currentState.toPrint, maxChars); 
			clear_area(posy, posx, 3, maxChars + 2);
			
			curs_set(0);
			update_hud();
			break;
		case 1:
			if( has_colors() ) change_color_popup(window);
			else show_message_top_left("This terminal does not have the capability for colors");
			break;
		case 2:
			if ( (currentState.chMask & A_REVERSE) == A_REVERSE ) currentState.chMask = currentState.chMask & !A_REVERSE;
			else currentState.chMask = currentState.chMask | A_REVERSE;
			wattrset(window->ptr, currentState.chMask);
			break;
		case 3:
			currentState.mode = (currentState.mode == STICKY)? NORMAL : STICKY;
			break;
		case 4:
			print_input_menu(posy, posx, maxChars + 2, defaultBorder,
					"Insert file name (current directory):", "Esc to cancel");
			read_input_echo(posy + 1, posx + 1, file_name, maxChars); 
			clear_area(posy, posx, 3, maxChars + 2);
			
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
			print_help_screen();
			break;
		case 7:
			end_screen();
			exit(0);
			break;
	}
	curs_set(0);
	return 0;
}

int main(int argc, char **argv){
	
	int startx, starty, dx, dy;
	int inp;
	int optionSelected;
	int highlight;
	Win *popupWin, *drawWin;

	/* initialize curses */

	init();
//
	if( has_colors() ){
		start_color();
	}
//

	//initialize drawWin
	//drawWin.cols = COLS - 2;
	//drawWin.lines = LINES - 2;
	//drawWin.xpos = 1;
	//drawWin.ypos = 1;
	//drawWin.borderSize = 0;
	//drawWin.ptr = newwin(drawWin.lines, drawWin.cols, drawWin.xpos, drawWin.ypos);
	drawWin = create_Win(1, 1, LINES - 2, COLS - 2);
	wrefresh(drawWin->ptr);

	//initialize popupWin
	//popupWin.cols = 40;
	//popupWin.lines = 10;
	//popupWin.xpos = COLS/2 - (popupWin.cols/2);
	//popupWin.ypos = LINES/2 - (popupWin.lines/2);
	//popupWin.borderSize = 1;
	//popupWin.ptr = newwin(popupWin.lines, popupWin.cols, popupWin.ypos, popupWin.xpos);
	
	popupWin = create_Win( LINES/2 - 5, COLS/2 - 20, 10, 40 );

	//keypad(stdscr, 1);
	keypad(stdscr, 1);
	keypad(drawWin->ptr, 1);
	keypad(popupWin->ptr, 1);

	starty = drawWin->lines / 2;
	startx = drawWin->cols / 2;
	dy = starty;
	dx = startx;

	highlight = 0;


	box(stdscr,0,0);
	update_hud();
	
	wmove(drawWin->ptr, starty, startx);
	setup_menu_popup(popupWin, "| MENU |", SIDE_LEFT, options, numOptions, SIDE_CENTER);
	highlight_menu_line(popupWin, highlight, false);
	highlight_menu_line(popupWin, highlight, true);

	for(;;){
		if(currentState.focus == 0){
			///show_message_top_left("before wgetch drawWin");
			inp = wgetch(drawWin->ptr);
			switch(inp){
				case KEY_F(1):
					end_screen();
					exit(0);
					break;
				case KEY_F(2):
					popupWin->hidden = 0;
					touchwin(popupWin->ptr);
					wrefresh(popupWin->ptr);
					currentState.focus = 1;
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
				case ENTER:
				case ESC:
				case KEY_BACKSPACE:
					currentState.mode = get_mode(currentState.mode, inp);
				default :
					if(inp >= 32 && inp <=136){
						currentState.toPrint[0] = inp;
						currentState.toPrint[1] = 0;
					}
					update_hud();
					break;
			}
			if( currentState.mode != NORMAL && currentState.focus == 0 ){
				if( currentState.mode == DELETE || ( currentState.mode == STICKY && inp == KEY_BACKSPACE ) ){
					mvwaddchstr(drawWin->ptr, dy, dx , emptyChar); 
				}
				else{
					if( currentState.mode == INSERT || (currentState.mode == STICKY && inp == ENTER) ) mvwaddstr(drawWin->ptr, dy, dx , currentState.toPrint);
				}
				wrefresh(drawWin->ptr);
			}
			wmove(drawWin->ptr, dy, dx);
		}
		if( currentState.focus == 1 ){
			//MENU navigation
			while( (optionSelected = option_picker(popupWin, numOptions, &highlight)) >= 0 ){
				if (handle_enter(drawWin, optionSelected) == 0){
					update_hud();
					break;
				}
			}
			if( optionSelected == -1 ){
				currentState.focus = 0;
				remove_window(popupWin);
				wmove(drawWin->ptr, dy, dx);
				curs_set(1);
			}
			if( optionSelected == -2 ) break;
		}
	} 
	end_screen();
	
	return 0;
}
