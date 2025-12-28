#ifndef WIN_WRAPPER_H
#define WIN_WRAPPER_H

#include <ncurses.h>

struct Win_t{
	WINDOW *ptr;
	int lines;
	int cols;
	int xpos;
	int ypos;
	int borderSize;
	int hidden;
};

typedef struct Win_t Win;

#endif
