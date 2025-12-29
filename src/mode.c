#include "mode.h"

#include "tuiWrapper.h"
#include <string.h>


MODE get_mode(MODE curr_mode, int key){
	if ( curr_mode == STICKY ) return STICKY;
	switch(key){
		case ESC:
			return NORMAL;
		case ENTER:
			return (curr_mode == INSERT)? NORMAL:INSERT;
		case KEY_BACKSPACE:
			return (curr_mode == DELETE)? NORMAL:DELETE;
		case KEY_TAB:
			return (curr_mode == SELECT)? NORMAL:SELECT;
		case KEY_DELETE:
			return (curr_mode == VISUAL)? NORMAL:VISUAL;
		default:
			return curr_mode;
	}
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
