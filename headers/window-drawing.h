#include "tuiWrapper.h"

void setup_menu_popup(Win *window, char *title, SIDE title_centering, char **options, int numOptions, SIDE text_centering);

void setup_input_menu(Win* win, WinBorder border, char* printPrompt, char* optional_message);

void setup_color_menu(Win* color_win);

void print_help_screen();

void highlight_menu_line(Win* window, int lineNum, bool highlight);
