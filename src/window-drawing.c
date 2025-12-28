#include "headers/window-drawing.h"

#ifndef LAST_PAIR
#define LAST_PAIR 127
#endif

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

void setup_color_menu(Win* color_win, char* fgHex, char* bgHex){
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

	mvwaddstr(color_win->ptr, posYHex, posLetterHex, fgHex);
	mvwaddstr(color_win->ptr, posYHex, posBackHex, bgHex);

	return;
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

void print_help_screen(){
	Win *helpWin = create_Win(10, 10, 31, 60);
	curs_set(0);

	mvwaddstr(helpWin->ptr, 1, 1, 
	"Welcome to my program, and to the help menu,\n\
 to start drawing you can press <Esc> to exit the menu\n\
 and enter the drawing screen. (after exiting this prompt)\n\
 To go back to the menu, press <F2> (in the drawing screen).\
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
