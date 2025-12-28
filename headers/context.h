#ifndef CONTEXT_H
#define CONTEXT_H

#include "headers/collection.h"
#include "headers/mode.h"
#include "headers/win-wrapper.h"

typedef struct context_t{
	State* state;
	Collection custom_colors;
	Collection color_pairs;
	Win* focus;
	Win* theDrawWin;
} Context;

void init_context( Context *app, State* state );

void end_context( Context *app );

#endif
