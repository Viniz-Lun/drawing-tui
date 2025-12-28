#ifndef MODE_H
#define MODE_H

#ifndef MAXINPUT
#define MAXINPUT 512
#endif

#ifndef NCURSES_H
	#define KEY_BACKSPACE 263
#endif

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
	int chMask;
} State;

MODE get_mode(MODE curr_mode, int key);

void get_mode_nstring(char* dest, MODE mode, int maxlen);

#endif
