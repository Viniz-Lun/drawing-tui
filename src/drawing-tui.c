/* drawing-tui.c */

#include <fcntl.h>
#include <ncurses.h>
#include <stdio.h>
#include <unistd.h>

#include "headers/custom-utils.h"
#include "headers/myColor.h"
#include "headers/tuiWrapper.h"
#include "headers/window-drawing.h"

#define ESC 27
#define ENTER 10
#define COLOR_FG_PREVIEW 255
#define COLOR_BG_PREVIEW 254

typedef enum {
	INSERT,
	DELETE,
	NORMAL,
	SELECT,
	VISUAL,
	STICKY,
} MODE;

typedef struct collection_t{
	void* colPointer;
	short sizeOfElement;
	int maxDim;
	int size;
} Collection;

typedef struct state_t{
	char toPrint[MAXINPUT];
	MODE mode;
	attr_t chMask;
} State;

typedef struct context_t{
	State* state;
	Collection custom_colors;
	Collection color_pairs;
	Win* focus;
	Win* theDrawWin;
} Context;

void add_element_to_collection(Collection* collection, void* element){
	if( collection->maxDim <= collection->size || collection->size < 0 ) return;
	memcpy((collection->colPointer + (collection->sizeOfElement * collection->size)), element, collection->sizeOfElement);
	collection->size++;
}

void end_context( Context *app ){
	end_screen();
	app->theDrawWin = NULL;
	app->focus = &baseScr;
	free(app->custom_colors.colPointer);
	free(app->color_pairs.colPointer);
}

void init(Context *app, State *state){
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
	
	//app->custom_colors = NULL;
	app->focus = &baseScr;
	app->state = state;
	app->theDrawWin = &baseScr;

	app->custom_colors.colPointer = malloc((255) * sizeof(Color));
	memset(app->custom_colors.colPointer, 0, 255 * sizeof(Color)); 
	app->custom_colors.maxDim = 255;
	app->custom_colors.size = 0;
	app->custom_colors.sizeOfElement = sizeof(Color);

	app->color_pairs.colPointer = malloc((126) * sizeof(Pair));
	memset(app->color_pairs.colPointer, 0, 126 * sizeof(Pair)); 
	app->color_pairs.maxDim = 126;
	app->color_pairs.size = 0;
	app->color_pairs.sizeOfElement = sizeof(Pair);

	state->toPrint[0] = '$'; 
	state->toPrint[1] = 0;
	state->mode = NORMAL;
	state->chMask = 0;
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

//TODO check if pair is effectively in the drawing
int write_pairs_in_file(int fd, int offset_start, Collection pairs){
	char stringToWrite[24];
	Pair* pairArray;

	lseek(fd, offset_start, SEEK_SET);

	pairArray = pairs.colPointer;

	for( int i = 0; i < pairs.size; i++ ){
		if( pairArray[i].pairNum != 0 ){
			snprintf(stringToWrite, 23, "%d:{%d,%d},", pairArray[i].pairNum, pairArray[i].fg.colorNum, pairArray[i].bg.colorNum);
			if( write( fd, stringToWrite, sizeof(char) * strlen(stringToWrite) ) < 0){
				perror("Error saving to file");
				show_message_top_left("Error saving pairs in file", NULL);
				return -1;
			}
		}
	} 
	write(fd, "\n", sizeof(char));
	return lseek(fd, 0, SEEK_CUR);
}

//TODO put check for if color is present in the drawing
int write_colors_in_file(int fd, int offset_start, Collection colors){
	char stringToWrite[16];
	char hex[7];
	Color* colorArray;

	lseek(fd, offset_start, SEEK_CUR);

	colorArray = colors.colPointer;

	for(int i = 0; i < colors.size; i ++){
		if( colorArray[i].colorNum != 0 ){
			RGB_to_hex(hex, colorArray[i].rgb);
			snprintf(stringToWrite, 15, "%d:%6s,",colorArray[i].colorNum, hex);
			if( write(fd, stringToWrite, strlen(stringToWrite) * sizeof(char)) < 0 ){
				perror("Error saving to file");
				show_message_top_left("Error saving colors in file", NULL);
				return -1;
			}
		}
	}
	write(fd, "\n", sizeof(char));
	return lseek(fd, 0, SEEK_CUR);
}

int save_drawing_to_file(Context *app, char *file_name){
	int fd;
	chtype *buffer;
	char stringExitMsg[32];
	int filePosition;
	
	buffer = (chtype*) malloc(app->theDrawWin->cols * sizeof(chtype) +1);
	
	fd = open(file_name, O_WRONLY | O_CREAT, 0644);
	if (fd < 0){
		strncpy(stringExitMsg, " Error opening/creating the file ", 32);
		perror(stringExitMsg);
		show_message_top_left(stringExitMsg, NULL);
		refresh();
		free(buffer);
		return 1;
	}

	filePosition = write_colors_in_file(fd, 0, app->custom_colors);
	if( filePosition < 0 )
		return -1;
	else{
		filePosition = write_pairs_in_file(fd, filePosition, app->color_pairs);
		if( filePosition < 0 ) return -1;
	}

	for(int i = 0; i < app->theDrawWin->lines; i++){
		mvwinchstr(app->theDrawWin->ptr, i, 0, buffer);
		write(fd, buffer, sizeof(chtype) * app->theDrawWin->cols);
		*buffer = '\n';
		write(fd, buffer, sizeof(chtype));
	}
	free(buffer);
	close(fd);

	sprintf(stringExitMsg, " Done saving file, size: %lu ",
			app->theDrawWin->cols * app->theDrawWin->lines * sizeof(char) + sizeof(char) * app->theDrawWin->lines );
	show_message_top_left(stringExitMsg, NULL);
	refresh();
	return 0;
}

int initialize_pairs_from_file(int fd, int offset_start, Context *app){
	char *firstLine, *token, *save_ptr, *ptr, *freePtr;
	char colors[256];
	char c;
	int i;
	short fgColor, bgColor, pairNum;
	Pair tempPair;
	Color* colorArray = app->custom_colors.colPointer;

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
		
		tempPair.pairNum = pairNum;

		i = get_color_index_from_num(fgColor, app->custom_colors.colPointer, app->custom_colors.maxDim);
		if( i < 0 ) tempPair.fg.colorNum = 1;
		else tempPair.fg = colorArray[i];

		i = get_color_index_from_num(bgColor, app->custom_colors.colPointer, app->custom_colors.maxDim);
		if( i < 0 ) tempPair.bg.colorNum = 0;
		else tempPair.bg = colorArray[i];

		init_pair(pairNum, fgColor, bgColor);

		add_element_to_collection(&app->color_pairs, &tempPair);

		token = strsep(&save_ptr, ",");

		if(save_ptr != NULL) token = strtok_r(NULL, ":", &save_ptr);
		else break;
	}
	free( freePtr );
	return lseek(fd, 0, SEEK_CUR);
}

int initialize_colors_from_file(int fd, int offset_start, Context *app){
	char *firstLine, *token, *save_ptr, *freePtr;
	char c;
	int i;
	short colorNum;
	RGB rgb;
	Color temp;

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

		temp.colorNum = colorNum;
		temp.rgb = rgb;

		init_color(colorNum, rgb.r, rgb.g, rgb.b);
		add_element_to_collection(&app->custom_colors, &temp);

		if(save_ptr != NULL) token = strtok_r(NULL, ":", &save_ptr);
		else break;
	}

	free( freePtr );
	return lseek(fd, 0, SEEK_CUR);
}

int load_image_from_file(Context *app, char *file_name){
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
	
	bufferPointer = (chtype*) malloc((app->theDrawWin->cols + 1) * sizeof(chtype));
	memset(bufferPointer, 0, (app->theDrawWin->cols+1) * sizeof(chtype));
	
	len = strlen(file_name);
	if( isCurse(file_name, len) ){

		size = sizeof(chtype);

		filePosition = initialize_colors_from_file(fd, 0, app);
		if(filePosition < 0 ||
				initialize_pairs_from_file(fd, filePosition, app) < 0 ){
			show_message_top_left(" Error initializing colors from file ", NULL);
			free( bufferPointer );
			close(fd);
			return -2;
		}
	}
	else{
		size = sizeof(char);
	}

	for(i = 0; i < app->theDrawWin->lines; i++){
		for(j = 0; j < app->theDrawWin->cols + 1; j++){
			nread = read(fd,&bufferPointer[j], size);
			if ( bufferPointer[j] == '\n' || nread <= 0)
				break;
			if( i == 0 && j == 0 ) clear_Win(app->theDrawWin);
		}
		if (nread >= 0) {
			bufferPointer[j] = 0;
			mvwaddchnstr(app->theDrawWin->ptr, i, 0, bufferPointer, j);
		}
		if (nread <= 0) break;
	}
	
	free( bufferPointer );
	close(fd);

	if (nread >= 0) {
		snprintf(stringExitMsg, 64, " Done loading file: %s ", file_name);
		touchwin(app->theDrawWin->ptr);
	}
	else {
		strncpy(stringExitMsg, "Error reading from file", 31);
	}

	show_message_top_left(stringExitMsg, NULL);
	refresh();
	return 0;
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

attr_t get_attr_off(attr_t current, attr_t toTurnOff){
	return current & ~toTurnOff;
}

attr_t get_attr_on(attr_t current, attr_t toTurnOn){
	return current | toTurnOn;
}

attr_t get_attr_with_color_pair(attr_t current, short pairNum){
	return ( current & ~COLOR_PAIR(PAIR_NUMBER(current)) ) | COLOR_PAIR(pairNum);
}

void set_or_create_pair_from_rgb(Context *app, RGB fg, RGB bg){
	Pair newPair;
	newPair.pairNum = get_pair_index_from_rgb(fg, bg,
			app->color_pairs.colPointer, app->color_pairs.maxDim);

	if( newPair.pairNum < 0 ){
		newPair = make_new_color_pair(
				fg, bg,
				app->custom_colors.colPointer, app->custom_colors.maxDim,
				app->color_pairs.colPointer, app->color_pairs.maxDim);

		add_element_to_collection(&app->color_pairs, &newPair);

		if( get_color_index_from_num(newPair.fg.colorNum,
					app->custom_colors.colPointer,
					app->custom_colors.maxDim) < 0 )
		{
			add_element_to_collection(&app->custom_colors, &newPair.fg);
		}
		if( get_color_index_from_num(newPair.bg.colorNum,
					app->custom_colors.colPointer,
					app->custom_colors.maxDim) < 0 )
		{
			add_element_to_collection(&app->custom_colors, &newPair.bg);
		}
	}// if newPair.pairNum < 0

	app->state->chMask = get_attr_with_color_pair(app->state->chMask, newPair.pairNum);

	wattrset(app->theDrawWin->ptr, app->state->chMask);

	update_hud(*app->state);

	return;
}

int change_color_popup(Context *app){
	Win* color_win;
	RGB rgb1, rgb2;
	Pair newPair;
	int optionSelected = 0;
	int isValid, confirm;
	short fgColorNum , bgColorNum;
	char hexCodeFg[7], hexCodeBg[7];
	int posLetterHex, posBackHex, posYHex;

	color_win = create_Win(10,10,10,30);

	if( PAIR_NUMBER(app->state->chMask) == 0 ){
		init_color(COLOR_FG_PREVIEW, 666, 666, 666);
		init_color(COLOR_BG_PREVIEW, 90, 90, 117);
		init_pair(LAST_PAIR, COLOR_FG_PREVIEW, COLOR_BG_PREVIEW);
	}

	color_content(COLOR_FG_PREVIEW, &rgb1.r, &rgb1.g, &rgb1.b);
	color_content(COLOR_BG_PREVIEW, &rgb2.r, &rgb2.g, &rgb2.b);

	RGB_to_hex(hexCodeFg , rgb1);
	RGB_to_hex(hexCodeBg , rgb2);

	setup_color_menu(color_win, hexCodeFg, hexCodeBg);

	posYHex = color_win->lines - color_win->borderSize - 1;
	posLetterHex = color_win->borderSize + 2;
	posBackHex = color_win->cols - color_win->borderSize - 7;
	
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
				break;
			case 1:
				confirm = read_input_echo(color_win, posYHex, posBackHex, hexCodeBg, 6);
				if( confirm ){
					pad_string_with_char_right(hexCodeBg, '0', 6);
					isValid = hex_to_RGB(hexCodeBg, &rgb2);
				}
				break;
			case 2:
				set_or_create_pair_from_rgb(app, rgb1, rgb2);
				break;
			default:
				break;
		}
		mvwaddstr(color_win->ptr, posYHex, posLetterHex, hexCodeFg);
		mvwaddstr(color_win->ptr, posYHex, posBackHex, hexCodeBg);

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

int handle_enter(Win *inputMenu, int optNum, Context* app){
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

			read_input_echo(inputMenu, 1, 1, app->state->toPrint, maxChars); 
			
			remove_window(inputMenu);
			update_hud(*app->state);
			break;
		case 1:
			if( has_colors() ) change_color_popup(app);
			else show_message_top_left("This terminal does not have the capability for colors", NULL);
			break;
		case 2:
			if ( (app->state->chMask & A_REVERSE) == A_REVERSE )
				app->state->chMask = get_attr_off(app->state->chMask, A_REVERSE);
			else
				app->state->chMask = get_attr_on(app->state->chMask, A_REVERSE);

			wattrset(app->theDrawWin->ptr, app->state->chMask);
			break;
		case 3:
			app->state->mode = (app->state->mode == STICKY)? NORMAL : STICKY;
			break;
		case 4:
			setup_input_menu(inputMenu, defaultBorder,
					"Insert file name (current directory):", "Esc to cancel"); 
			show_Win(inputMenu);

			read_input_echo(inputMenu, 1, 1, file_name, maxChars); 
			
			remove_window(inputMenu);
			
			if(file_name[0] != '\0'){
				load_image_from_file(app, file_name);
				return 1;
			}
			break;
		case 5:
			setup_input_menu(inputMenu, defaultBorder,
					"Insert file name (current directory):", "Esc to cancel"); 
			show_Win(inputMenu);

			read_input_echo(inputMenu, 1, 1, file_name, maxChars); 
			
			remove_window(inputMenu);
			
			if(file_name[0] != '\0'){
				save_drawing_to_file(app, file_name);
			}
			break;
		case 6:
			print_help_screen();
			break;
		case 7:
			end_context(app);
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
	MODE action;
	Win *popupWin, *drawWin, *inputMenu;
	Context app;
	State currentState;
	MEVENT mouse;

	/* initialize curses */

	init(&app, &currentState);


	///
	mousemask(BUTTON1_CLICKED, NULL);
	mouseinterval(0);
	///
	if( has_colors() ){
		start_color();
	}

	//initialize drawWin
	drawWin = create_Win(1, 1, LINES - 2, COLS - 2);

	app.theDrawWin = drawWin;

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

	app.focus = popupWin;

	for(;;){
		if(app.focus == drawWin){
			inp = wgetch(drawWin->ptr);
			
			if( inp == KEY_F(2) ){
				popupWin->hidden = 0;
				touchwin(popupWin->ptr);
				wrefresh(popupWin->ptr);
				app.focus = popupWin;
				break;
			}
			
			switch(inp){
				case KEY_MOUSE:
					getmouse(&mouse);
					if( mouse.bstate & BUTTON1_CLICKED ){
						if( !(mouse.x > (drawWin->xpos + drawWin->cols) ||
								mouse.x < drawWin->xpos || mouse.y < drawWin->ypos ||
								mouse.y > (drawWin->ypos + drawWin->lines)) ){
							dx = mouse.x - drawWin->xpos;
							dy = mouse.y - drawWin->ypos;
						}
					}
					break;
				case KEY_F(1):
					end_context(&app);
					exit(0);
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
		}// focus == drawWin
		if( app.focus == popupWin ){
			//MENU navigation
			optionSelected = option_picker(popupWin, numOptions, &highlight);
			while( optionSelected >= 0 ){
				if( handle_enter(inputMenu, optionSelected, &app) == 0 ){
					update_hud(currentState);
					optionSelected = option_picker(popupWin, numOptions, &highlight);
				}
				else{
					break;
				}
			}
			if( optionSelected == -2 ){
				end_context(&app);
				exit(0);
			}
			app.focus = drawWin;
			remove_window(popupWin);
			wmove(drawWin->ptr, dy, dx);
			curs_set(1);
		}// focus == popupWin
	} 
	end_context(&app);
	
	return 0;
}
