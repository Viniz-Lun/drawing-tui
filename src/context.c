#include "headers/context.h"
#include "headers/tuiWrapper.h"

void init_context(Context *app, State* state){
	app->focus = &baseScr;
	app->state = state;
	app->theDrawWin = &baseScr;

	initialize_collection(&(app->custom_colors), 255, sizeof(Color));

	initialize_collection(&(app->color_pairs), 126, sizeof(Pair));

	app->custom_colors.size = init_base_colors(app->custom_colors.colPointer, app->custom_colors.maxDim);

	Pair defaultPair;
	defaultPair.pairNum = 0;
	Color temp = get_color_from_num(COLOR_WHITE, app->custom_colors.colPointer, app->custom_colors.maxDim);
	if( temp.colorNum != -1 ){
		defaultPair.fg = temp;
		temp = get_color_from_num(COLOR_BLACK, app->custom_colors.colPointer, app->custom_colors.maxDim);
		if( temp.colorNum != -1 ){
			defaultPair.bg = temp;
			add_element_to_collection(&app->color_pairs, &defaultPair);
		}
	}
	
	init_pair(defaultPair.pairNum, COLOR_WHITE, COLOR_BLACK);

	state->toPrint[0] = '$'; 
	state->toPrint[1] = 0;
	state->mode = NORMAL;
	state->chMask = COLOR_PAIR(0);
}

void end_context( Context *app ){
	app->theDrawWin = NULL;
	app->focus = &baseScr;
	free(app->custom_colors.colPointer);
	free(app->color_pairs.colPointer);
}
