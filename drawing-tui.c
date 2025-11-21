/* triangle.c */
#include <curses.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "RGB.h"
#include "custom-utils.h"
#include "tuiWrapper.h"

#define ESC 27
#define ENTER 10
#define COLOR_FG_PREVIEW 255
#define COLOR_BG_PREVIEW 254
#define FIRST_PAIR 1
#define LAST_PAIR 127

typedef enum {
	INSERT,
	DELETE,
	NORMAL,
	SELECT,
	VISUAL,
	STICKY,
} MODE;

typedef struct state_t{
	char toPrint[MAXINPUT];
	MODE mode;
	attr_t chMask;
	Win* focus;
	Win* theDrawWin;
} State;

//typedef struct context_t{
//	State state;
//	WinList allWins;
//	int* colors;
//	int* color_pairs;
//} Context;

void init(State *state){
	initscr();
	cbreak();
	noecho();
	set_escdelay(50);
	
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

	set_Win_list( cons(&baseScr, NULL) );
	
	state->toPrint[0] = '$'; 
	state->toPrint[1] = 0;
	state->mode = NORMAL;
	state->chMask = 0;
	state->focus = &baseScr;
	state->theDrawWin = &baseScr;
}

MODE get_mode(MODE curr_mode, int key){
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

void show_message_top_left(char* message, int *value){
	mvaddch(0, 0, ACS_ULCORNER);
	mvhline(0, 1, ACS_HLINE, COLS - 2);
	mvaddstr(0, 2, message);
	if( value != NULL ){
		mvprintw(0, strlen(message), "%d", *value);
	}
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
			strncpy(dest, "DELETE", maxlen);
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

void update_hud(State currentState){
	char string[MAXINPUT];
	char modeString[8];

	mvhline(LINES - 1, 1, ACS_HLINE, COLS - 2);

	strncpy(string, " PALETTE:  ", MAXINPUT);
	mvaddstr(LINES - 1, get_xpos_for_string_window(baseScr, string, SIDE_LEFT, 1), string);
	mvaddch( LINES - 1, get_xpos_for_string_size(LINES, "", SIDE_LEFT, strlen(string) - 1), '$' | currentState.chMask );

	sprintf(string, " Brush: '%s' ", currentState.toPrint);
	mvaddstr(LINES - 1, get_xpos_for_string_size(COLS, string, SIDE_CENTER, 0), string);

	get_mode_nstring(modeString, currentState.mode, 7);
	sprintf(string, "[ %s ]", modeString);

	mvaddstr(LINES - 1, get_xpos_for_string_size(COLS, string, SIDE_RIGHT, 1), string);
	refresh();
}

void setup_menu_popup(Win *window, char *title, SIDE title_centering, char **options, int numOptions, SIDE text_centering){
	int y, x;
	border_Win(window, defaultBorder);
	
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


void setup_input_menu(Win* win, WinBorder border, char* printPrompt, char* optional_message){
	int centerXpos, len, inputChar;
	char prompt[200];
	
	clear_Win(win);
	border_Win(win, border);

	strncpy(prompt, printPrompt, 200);
	len = strlen(prompt);

	if (len >= win->cols) prompt[win->cols] = '\0';

	centerXpos = get_xpos_for_string_window( *win, prompt, SIDE_CENTER, 0),
	mvwaddstr(win->ptr, 0, centerXpos, prompt);

	if(optional_message != NULL){
		strncpy(prompt, optional_message, 200);
		len = strlen(prompt);

		if (len >= win->cols) prompt[win->cols] = '\0';

		centerXpos = get_xpos_for_string_window( *win, prompt, SIDE_CENTER, 0),
		mvwaddstr(win->ptr, win->lines - 1, centerXpos, optional_message);
	}
	
	return;
}

//TO-DO
//proto function
int write_pairs_in_file(int fd, int offset_start, int* pairs){
	char buffer[512];
	short bgColorNum;
	short fgColorNum;
	RGB rgb;
	char fgHexCode[7];
	char bgHexCode[7];
	int i, j;

	for( i = 0; i < pairs; i++ ){
		if( i != 0 ){
			strcpy(buffer, ",");
			write(fd, buffer, sizeof(char));
		}
		pair_content(pairs[i], &fgColorNum, &bgColorNum);

		color_content(fgColorNum, &rgb.r, &rgb.g, &rgb.b);
		RGB_to_hex(fgHexCode, rgb);

		color_content(bgColorNum, &rgb.r, &rgb.g, &rgb.b);
		RGB_to_hex(bgHexCode, rgb);

		sprintf(buffer, "%d:{%s,%s}", pairs[i], fgHexCode, bgHexCode);
		write(fd, buffer, sizeof(char) * strlen(buffer));
	} 
	write(fd, "\n", sizeof(char));
	return lseek(fd, 0, SEEK_CUR);
}
//TO-DO
int write_colors_in_file(int fd, int offset_start, int* colors);

int save_drawing_to_file(Win *window, char *file_name, State *currentState){
	int fd;
	chtype *buffer;
	char stringExitMsg[32];
	//int colors[256];
	//int pairs[127];
	int filePosition;
	
	buffer = (chtype*) malloc(window->cols * sizeof(chtype) +1);
	
	fd = open(file_name, O_WRONLY | O_CREAT, 0644);
	if (fd < 0){
		strncpy(stringExitMsg, " Error opening/creating the file ", 32);
		perror(stringExitMsg);
		show_message_top_left(stringExitMsg, NULL);
		refresh();
		free(buffer);
		return 1;
	}

	//filePosition = write_colors_in_file(fd, 0, pairs);
	//filePosition = write_pairs_in_file(fd, filePosition, colors);

	for(int i = 0; i < window->lines; i++){
		mvwinchstr(window->ptr, i, 0, buffer);
		write(fd, buffer, sizeof(chtype) * window->cols);
		*buffer = '\n';
		write(fd, buffer, sizeof(chtype));
	}
	free(buffer);
	close(fd);

	sprintf(stringExitMsg, " Done saving file, size: %lu ", window->cols * window->lines * sizeof(char) + sizeof(char) * window->lines );
	show_message_top_left(stringExitMsg, NULL);
	refresh();
	return 0;
}

int initialize_pairs_from_file(int fd, int offset_start, State *currentState){
	char *firstLine, *token, *save_ptr, *ptr, *freePtr;
	char colors[256];
	char c;
	int i;
	short fgColor, bgColor, pairNum;

	lseek(fd, offset_start, SEEK_SET);
	for( i = 0; (read(fd, &c, sizeof(char)) > 0) && c != '\n'; i++ );

	firstLine = (char*) malloc(sizeof(char) * (i + 1));
	if( firstLine == NULL ){ 
		perror("Errore malloc");
		return -1;
	}
	freePtr = firstLine;

	lseek(fd, offset_start, SEEK_SET);
	for( i = 0; (read(fd, &c, sizeof(char)) > 0) && c != '\n'; i++ ){
		firstLine[i] = c;
	}
	firstLine[i] = '\0';
	
	token = strtok_r(firstLine, ":", &save_ptr);

	while( token != NULL ){
		pairNum = atoi( token );

		token = strsep(&save_ptr, "{");

		token = strtok_r(NULL, "}", &save_ptr);
		strcpy(colors, token);

		ptr = colors;
		token = strsep( &ptr,  ",");

		fgColor = atoi(token);

		token = ptr;
		bgColor = atoi(token);
		
		init_pair(pairNum, fgColor, bgColor);

		token = strsep(&save_ptr, ",");


		if(save_ptr != NULL) token = strtok_r(NULL, ":", &save_ptr);
		else break;
	}
	free( freePtr );
	return lseek(fd, 0, SEEK_CUR);
}

int initialize_colors_from_file(int fd, int offset_start, State *currentState){
	char *firstLine, *token, *save_ptr, *freePtr;
	char colors[256];
	char c;
	int i;
	short colorNum;
	RGB rgb;

	lseek(fd, offset_start, SEEK_SET);
	for( i = 0; (read(fd, &c, sizeof(char)) > 0) && c != '\n'; i++ );

	firstLine = (char*) malloc(sizeof(char) * (i + 1));
	if( firstLine == NULL ){ 
		perror("Errore malloc");
		return -1;
	}
	freePtr = firstLine;

	lseek(fd, offset_start, SEEK_SET);
	for( i = 0; (read(fd, &c, sizeof(char)) > 0) && c != '\n'; i++ ){
		firstLine[i] = c;
	}
	firstLine[i] = '\0';
	
	token = strtok_r(firstLine, ":", &save_ptr);
	while( token != NULL ){
		colorNum = atoi( token );
		token = strtok_r(NULL, ",", &save_ptr);
		
		hex_to_RGB(token, &rgb);

		init_color(colorNum, rgb.r, rgb.g, rgb.b);

		if(save_ptr != NULL) token = strtok_r(NULL, ":", &save_ptr);
		else break;
	}

	free( freePtr );
	return lseek(fd, 0, SEEK_CUR);
}

int load_image_from_file(Win *window, char *file_name, State *currentState){
	int fd;
	int i, j, nread, len, filePosition;
	char stringExitMsg[65];
	chtype *bufferPointer;
	size_t size;

	fd = open(file_name, O_RDONLY);
	if (fd < 0){
		show_message_top_left(" Error opening the file ", NULL);
		refresh();
		return -1;
	}
	
	bufferPointer = (chtype*) malloc((window->cols + 1) * sizeof(chtype));
	memset(bufferPointer, 0, (window->cols+1) * sizeof(chtype));
	
	len = strlen(file_name);
	if( isCurse(file_name, len) ){

		size = sizeof(chtype);

		filePosition = initialize_colors_from_file(fd, 0, currentState);
		if(filePosition < 0 ||
				initialize_pairs_from_file(fd, filePosition, currentState) < 0 ){
			show_message_top_left(" Error initializing colors from file ", NULL);
			free( bufferPointer );
			close(fd);
			return -2;
		}
	}
	else{
		size = sizeof(char);
	}

	for(i = 0; i < window->lines; i++){
		for(j = 0; j < window->cols + 1; j++){
			nread = read(fd,&bufferPointer[j], size);
			if ( bufferPointer[j] == '\n' || nread <= 0)
				break;
			if( i == 0 && j == 0 ) clear_Win(window);
		}
		if (nread >= 0) {
			bufferPointer[j] = 0;
			mvwaddchnstr(window->ptr, i, 0, bufferPointer, j);
		}
		if (nread <= 0) break;
	}
	
	free( bufferPointer );
	close(fd);

	if (nread >= 0) {
		snprintf(stringExitMsg, 64, " Done loading file: %s ", file_name);
		touchwin(window->ptr);
	}
	else {
		strncpy(stringExitMsg, "Error reading from file", 31);
	}

	show_message_top_left(stringExitMsg, NULL);
	refresh();
	return 0;
}

void print_help_screen(){
	Win *helpWin = create_Win(10, 10, 30, 60);
	curs_set(0);

	mvwaddstr(helpWin->ptr, 1, 1, 
	"Welcome to my program, and to the help menu,\n\
 to start drawing you can press <Esc> to exit the menu\n\
 and enter the drawing screen. (after exiting this prompt)\n\
 MODES:\n\
 There are currently 4 modes implemented, which are:\n\
 NORMAL:\n\
 - Key: <Esc>\n\
 - The mode used to move around the screen without\n\
 modifying the drawing.\n\
 INSERT:\n\
 - Key: <Enter>\n\
 - The mode to actually draw on the screen, \n\
 whenever you move the cursor you place the content(s)\n\
 of the brush at the current position.\n\
 DELETE:\n\
 - Key: <BackSpace>\n\
 - The mode to put an empty space wherever your\n\
 cursor moves.\n\
 STICKY:\n\
 - Key: Menu option\n\
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

void setup_color_menu(Win* color_win){
	char *color_options[] ={
		"Choose letter color",
		"Choose background color",
		"Confirm",
	};
	int posLetterHex, posBackHex, posYHex;

	setup_menu_popup(color_win, "| Color picker |", SIDE_CENTER, color_options, 3, SIDE_LEFT);

	wattron(color_win->ptr, COLOR_PAIR(LAST_PAIR));
	mvwaddstr( color_win->ptr, color_win->lines /2, color_win->cols/2 - 2, "$$$$");
	wattroff(color_win->ptr, COLOR_PAIR(LAST_PAIR));

	posYHex = color_win->lines - color_win->borderSize - 1;
	posLetterHex = color_win->borderSize + 2;
	posBackHex = color_win->cols - color_win->borderSize - 7;

	mvwaddch(color_win->ptr, posYHex, posLetterHex - 1, '#');
	mvwaddch(color_win->ptr, posYHex, posBackHex - 1, '#');

	return;
}

attr_t get_attr_off(attr_t current, attr_t toTurnOff){
	return current & ~toTurnOff;
}

attr_t get_attr_on(attr_t current, attr_t toTurnOff){
	return current | toTurnOff;
}

int change_color_popup(State *currentState){
	Win* color_win;
	RGB rgb1, rgb2;
	int optionSelected = 0;
	int isValid, confirm;
	short newPair;
	char hexCodeFg[7], hexCodeBg[7];
	int posLetterHex, posBackHex, posYHex;

	color_win = create_Win(10,10,10,30);

	if( PAIR_NUMBER(currentState->chMask) == 0 ){
		init_color(COLOR_FG_PREVIEW, 666, 666, 666);
		init_color(COLOR_BG_PREVIEW, 90, 90, 117);
		init_pair(LAST_PAIR, COLOR_FG_PREVIEW, COLOR_BG_PREVIEW);
	}

	setup_color_menu(color_win);

	color_content(COLOR_FG_PREVIEW, &rgb1.r, &rgb1.g, &rgb1.b);
	color_content(COLOR_BG_PREVIEW, &rgb2.r, &rgb2.g, &rgb2.b);

	posYHex = color_win->lines - color_win->borderSize - 1;
	posLetterHex = color_win->borderSize + 2;
	posBackHex = color_win->cols - color_win->borderSize - 7;
	
	RGB_to_hex(hexCodeFg , rgb1);
	RGB_to_hex(hexCodeBg , rgb2);

	mvwaddstr(color_win->ptr, posYHex, posLetterHex, hexCodeFg);
	mvwaddstr(color_win->ptr, posYHex, posBackHex, hexCodeBg);

	optionSelected = option_picker(color_win, 3, &optionSelected);
	while( optionSelected >= 0 ){
		curs_set(1);

		switch(optionSelected){
			case 0:
				confirm = read_input_echo(color_win, posYHex, posLetterHex, hexCodeFg, 6);
				if( confirm ){
					pad_string_with_char_right(hexCodeFg, '0', 6);
					isValid = hex_to_RGB(hexCodeFg, &rgb1);
				}
				mvwaddstr(color_win->ptr, posYHex, posLetterHex, hexCodeFg);
				break;
			case 1:
				confirm = read_input_echo(color_win, posYHex, posBackHex, hexCodeBg, 6);
				if( confirm ){
					pad_string_with_char_right(hexCodeBg, '0', 6);
					isValid = hex_to_RGB(hexCodeBg, &rgb2);
				}
				mvwaddstr(color_win->ptr, posYHex, posBackHex, hexCodeBg);
				break;
			case 2: 
				newPair = PAIR_NUMBER(currentState->chMask);
				currentState->chMask = get_attr_off(currentState->chMask, COLOR_PAIR(newPair));

				newPair = make_new_color_pair(newPair + 1, rgb1, rgb2, NULL);

				currentState->chMask = get_attr_on(currentState->chMask, COLOR_PAIR(newPair));
				wattrset(currentState->theDrawWin->ptr, currentState->chMask);
				break;
			default:
				break;
		}

		if( confirm ){
			if( isValid ){
				mvwhline( color_win->ptr, color_win->lines /2 + 1, color_win->cols/2 - 7, ' ', 14);
				init_color(COLOR_FG_PREVIEW, rgb1.r, rgb1.g, rgb1.b);
				init_color(COLOR_BG_PREVIEW, rgb2.r, rgb2.g, rgb2.b);
				init_pair(LAST_PAIR, COLOR_FG_PREVIEW, COLOR_BG_PREVIEW);
			}
			else{
				mvwaddstr( color_win->ptr, color_win->lines /2 + 1, color_win->cols/2 - 7, "Hex not valid");
			}
		}

		wrefresh(color_win->ptr);

		optionSelected = option_picker(color_win, 3, &optionSelected);
	}//while

	delete_Win(color_win);
	return 0;
}

int handle_enter(Win *inputMenu,int optNum, State *currentState){
	int fd;
	char file_name[MAXINPUT] = {0};
	int posy, posx;
	int maxChars;
	int width;
	
	width = inputMenu->cols - 2;
	maxChars = min(MAXINPUT - 1, width );

	switch(optNum){
		case 0:
			setup_input_menu(inputMenu, defaultBorder, "Write string to use:", NULL); 
			show_Win(inputMenu);

			curs_set(1);
			read_input_echo(inputMenu, 1, 1, currentState->toPrint, maxChars); 
			curs_set(0);
			
			remove_window(inputMenu);
			update_hud(*currentState);
			break;
		case 1:
			if( has_colors() ) change_color_popup(currentState);
			else show_message_top_left("This terminal does not have the capability for colors", NULL);
			break;
		case 2:
			if ( (currentState->chMask & A_REVERSE) == A_REVERSE )
				currentState->chMask = get_attr_off(currentState->chMask, A_REVERSE);
			else
				currentState->chMask = get_attr_on(currentState->chMask, A_REVERSE);

			wattrset(currentState->theDrawWin->ptr, currentState->chMask);
			break;
		case 3:
			currentState->mode = (currentState->mode == STICKY)? NORMAL : STICKY;
			break;
		case 4:
			setup_input_menu(inputMenu, defaultBorder,
					"Insert file name (current directory):", "Esc to cancel"); 
			show_Win(inputMenu);

			curs_set(1);
			read_input_echo(inputMenu, 1, 1, file_name, maxChars); 
			curs_set(0);
			
			remove_window(inputMenu);
			
			if(file_name[0] != '\0'){
				load_image_from_file(currentState->theDrawWin, file_name, currentState);
				return 1;
			}
			break;
		case 5:
			setup_input_menu(inputMenu, defaultBorder,
					"Insert file name (current directory):", "Esc to cancel"); 
			show_Win(inputMenu);

			curs_set(1);
			read_input_echo(inputMenu, 1, 1, file_name, maxChars); 
			curs_set(0);
			
			remove_window(inputMenu);
			
			if(file_name[0] != '\0'){
				save_drawing_to_file(currentState->theDrawWin, file_name, currentState);
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
	Win *popupWin, *drawWin, *inputMenu;
	State currentState;
	MODE action;

	/* initialize curses */

	init(&currentState);

	if( has_colors() ){
		start_color();
	}

	//initialize drawWin
	drawWin = create_Win(1, 1, LINES - 2, COLS - 2);

	currentState.theDrawWin = drawWin;

	wrefresh(drawWin->ptr);

	//initialize popupWin
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

	popupWin = create_Win( LINES/2 - 5, COLS/2 - 20, 10, 40 );
	
	//initialize inputMenu
	inputMenu = create_Win( LINES * 3 / 4 , COLS*1/3, 3, COLS*1/3 );

	keypad(baseScr.ptr, 1);
	keypad(drawWin->ptr, 1);
	keypad(popupWin->ptr, 1);
	keypad(inputMenu->ptr, 1);

	starty = drawWin->lines / 2;
	startx = drawWin->cols / 2;
	dy = starty;
	dx = startx;

	highlight = 0;

	//Not exactly a border but more of a style choice, so no use of border_Win.
	box(stdscr,0,0);
	update_hud(currentState);
	
	wmove(drawWin->ptr, starty, startx);

	setup_menu_popup(popupWin, "| MENU |", SIDE_LEFT, options, numOptions, SIDE_CENTER);
	highlight_menu_line(popupWin, highlight, true);

	currentState.focus = popupWin;

	for(;;){
		if(currentState.focus == drawWin){
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
					currentState.focus = popupWin;
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
					update_hud(currentState);
					break;
			}
			if( currentState.focus == drawWin ){
				action = (currentState.mode == STICKY)? get_mode(NORMAL, inp): currentState.mode;
				switch( action ){
					case DELETE:
						mvwaddchnstr(drawWin->ptr, dy, dx , &emptyChar, 1); 
						break;
					case INSERT:
						mvwaddstr(drawWin->ptr, dy, dx , currentState.toPrint);
						break;
					case NORMAL:
					default:
						break;
				}
				wmove(drawWin->ptr, dy, dx);
				wrefresh(drawWin->ptr);
			}
		}// focus == drawWin
		if( currentState.focus == popupWin ){
			//MENU navigation
			optionSelected = option_picker(popupWin, numOptions, &highlight);
			while( optionSelected >= 0 ){
				if( handle_enter(inputMenu, optionSelected, &currentState) == 0 ){
					update_hud(currentState);
					optionSelected = option_picker(popupWin, numOptions, &highlight);
				}
				else{
					break;
				}
			}
			if( optionSelected == -2 ) break;
			currentState.focus = drawWin;
			remove_window(popupWin);
			wmove(drawWin->ptr, dy, dx);
			curs_set(1);
		}// focus == popupWin
	} 
	end_screen();
	
	return 0;
}
