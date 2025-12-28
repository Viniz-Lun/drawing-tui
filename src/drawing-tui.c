/* drawing-tui.c */

#include <unistd.h>
#include <stdlib.h>
#include <string.h>

#include "headers/custom-utils.h"
#include "headers/myColor.h"
#include "headers/window-drawing.h"
#include "headers/curse-files.h"

#define COLOR_FG_PREVIEW 255
#define COLOR_BG_PREVIEW 254

void end_program(Context* app){
	end_screen();
	end_context(app);
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
	int index;

	color_win = create_Win(10,10,10,30);

	index = get_pair_index_from_pair_num(PAIR_NUMBER(app->state->chMask), app->color_pairs.colPointer, app->color_pairs.maxDim);
	if( index < 0 ){
		rgb1 = (RGB){666,666,666};
		rgb2 = (RGB){0,0,0};
	}
	else{
		newPair = *(Pair*)get_element_at_index(&app->color_pairs, index);
		rgb1 = newPair.fg.rgb;
		rgb2 = newPair.bg.rgb;
		
	}

	init_color(COLOR_FG_PREVIEW, rgb1.r, rgb1.g, rgb1.b);
	init_color(COLOR_BG_PREVIEW, rgb2.r, rgb2.g, rgb2.b);
	init_pair(LAST_PAIR, COLOR_FG_PREVIEW, COLOR_BG_PREVIEW);

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
			end_program(app);
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

	init_tui();
	init_context(&app, &currentState);

	///
	mousemask(BUTTON1_CLICKED, NULL);
	mouseinterval(0);
	///

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
				continue;
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
					end_program(&app);
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
				default :
					currentState.mode = get_mode(currentState.mode, inp);
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
				end_program(&app);
				exit(0);
			}
			app.focus = drawWin;
			remove_window(popupWin);
			wmove(drawWin->ptr, dy, dx);
			curs_set(1);
		}// focus == popupWin
	} 
	end_program(&app);
	
	return 0;
}
