#include "mode.h"

#include "tuiWrapper.h"
#include <string.h>


MODE get_mode(MODE curr_mode, int key){
	if ( curr_mode == STICKY ) return STICKY;
	switch(key){
		case ESC:
			return HOVER;
		case 'i':
			return (curr_mode == SELECT)? SELECT:TYPING;
		case KEY_BACKSPACE:
			return (curr_mode == DELETE)? HOVER:DELETE;
		case KEY_TAB:
			return (curr_mode == SELECT)? HOVER:SELECT;
		case ENTER:
			return (curr_mode == PLACE)? HOVER:PLACE;
		case 'v':
			return (curr_mode == VISUAL)? HOVER:VISUAL;
		default:
			return curr_mode;
	}
}

void get_mode_nstring(char* dest, MODE mode, int maxlen){
	switch(mode){
		case HOVER:
			strncpy(dest, "HOVER",maxlen);
			break;
		case TYPING:
			strncpy(dest, "TYPING",maxlen);
			break;
		case SELECT:
			strncpy(dest, "SELECT",maxlen);
			break;
		case DELETE:
			strncpy(dest, "DELETE", maxlen);
			break;
		case PLACE:
			strncpy(dest, "PLACE",maxlen);
			break;
		case STICKY:
			strncpy(dest, "STICKY", maxlen);
			break;
		case VISUAL:
			strncpy(dest, "VISUAL", maxlen);
			break;
		default:
			strncpy(dest, "HOVER",maxlen);
			break;
	}
}
