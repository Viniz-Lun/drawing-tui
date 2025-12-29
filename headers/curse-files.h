#ifndef CURSE_FILES_H
#define CURSE_FILES_H

#include "context.h"

#define isCurse(A,LEN) A[LEN-1] == 'e' && A[LEN-2] == 's' && A[LEN-3] == 'r' && A[LEN-4] == 'u' && A[LEN-5] == 'c' && A[LEN-6] == '.'

int initialize_pairs_from_file(int fd, int offset_start, Context *app);

int initialize_colors_from_file(int fd, int offset_start, Context *app);

int write_pairs_in_file(int fd, int offset_start, Collection pairs);

int write_colors_in_file(int fd, int offset_start, Collection colors);

int load_image_from_file(Context *app, char *file_name);

int save_drawing_to_file(Context *app, char *file_name);

void show_message_top_left(char* message, int *value);

#endif
