#include "headers/curse-files.h"
#include "headers/tuiWrapper.h"

#include <unistd.h>
#include <fcntl.h>

int initialize_pairs_from_file(int fd, int offset_start, Context *app){
	char *firstLine, *token, *save_ptr, *ptr, *freePtr;
	char colors[256];
	char c;
	int i;
	short fgColor, bgColor, pairNum;
	Pair tempPair;
	Color* colorArray = app->custom_colors.colPointer;

	lseek(fd, offset_start, SEEK_SET);
	for( i = 0; (read(fd, &c, sizeof(char)) > 0) && c != '\n'; i++ );

	firstLine = (char*) malloc(sizeof(char) * (i + 1));
	if( firstLine == NULL ){ 
		perror("Errore malloc");
		return -1;
	}
	freePtr = firstLine;

	lseek(fd, offset_start, SEEK_SET);
	for( i = 0; (read(fd, &c, sizeof(char)) > 0) && c != '\n'; i++ ){
		firstLine[i] = c;
	}
	firstLine[i] = '\0';
	
	token = strtok_r(firstLine, ":", &save_ptr);

	while( token != NULL ){
		pairNum = atoi( token );

		token = strsep(&save_ptr, "{");

		token = strtok_r(NULL, "}", &save_ptr);
		strcpy(colors, token);

		ptr = colors;
		token = strsep( &ptr,  ",");

		fgColor = atoi(token);

		token = ptr;
		bgColor = atoi(token);
		
		tempPair.pairNum = pairNum;

		i = get_color_index_from_num(fgColor, app->custom_colors.colPointer, app->custom_colors.maxDim);
		if( i < 0 ) tempPair.fg.colorNum = 1;
		else tempPair.fg = colorArray[i];

		i = get_color_index_from_num(bgColor, app->custom_colors.colPointer, app->custom_colors.maxDim);
		if( i < 0 ) tempPair.bg.colorNum = 0;
		else tempPair.bg = colorArray[i];

		init_pair(pairNum, fgColor, bgColor);

		add_element_to_collection(&app->color_pairs, &tempPair);

		token = strsep(&save_ptr, ",");

		if(save_ptr != NULL) token = strtok_r(NULL, ":", &save_ptr);
		else break;
	}
	free( freePtr );
	return lseek(fd, 0, SEEK_CUR);
}

int initialize_colors_from_file(int fd, int offset_start, Context *app){
	char *firstLine, *token, *save_ptr, *freePtr;
	char c;
	int i;
	short colorNum;
	RGB rgb;
	Color temp;

	lseek(fd, offset_start, SEEK_SET);
	for( i = 0; (read(fd, &c, sizeof(char)) > 0) && c != '\n'; i++ );

	firstLine = (char*) malloc(sizeof(char) * (i + 1));
	if( firstLine == NULL ){ 
		perror("Errore malloc");
		return -1;
	}
	freePtr = firstLine;

	lseek(fd, offset_start, SEEK_SET);
	for( i = 0; (read(fd, &c, sizeof(char)) > 0) && c != '\n'; i++ ){
		firstLine[i] = c;
	}
	firstLine[i] = '\0';

	zero_out_collection_contents(&app->color_pairs);
	zero_out_collection_contents(&app->custom_colors);
	
	token = strtok_r(firstLine, ":", &save_ptr);
	while( token != NULL ){
		colorNum = atoi( token );
		token = strtok_r(NULL, ",", &save_ptr);
		
		hex_to_RGB(token, &rgb);

		temp.colorNum = colorNum;
		temp.rgb = rgb;

		init_color(colorNum, rgb.r, rgb.g, rgb.b);
		add_element_to_collection(&app->custom_colors, &temp);

		if(save_ptr != NULL) token = strtok_r(NULL, ":", &save_ptr);
		else break;
	}

	free( freePtr );
	return lseek(fd, 0, SEEK_CUR);
}


//TODO check if pair is effectively in the drawing
int write_pairs_in_file(int fd, int offset_start, Collection pairs){
	char stringToWrite[24];
	Pair* pairArray;

	lseek(fd, offset_start, SEEK_SET);

	pairArray = pairs.colPointer;

	for( int i = 0; i < pairs.size; i++ ){
		snprintf(stringToWrite, 23, "%d:{%d,%d},", pairArray[i].pairNum, pairArray[i].fg.colorNum, pairArray[i].bg.colorNum);
		if( write( fd, stringToWrite, sizeof(char) * strlen(stringToWrite) ) < 0){
			perror("Error saving to file");
			show_message_top_left("Error saving pairs in file", NULL);
			return -1;
		}
	} 
	write(fd, "\n", sizeof(char));
	return lseek(fd, 0, SEEK_CUR);
}

//TODO put check for if color is present in the drawing
int write_colors_in_file(int fd, int offset_start, Collection colors){
	char stringToWrite[16];
	char hex[7];
	Color* colorArray;

	lseek(fd, offset_start, SEEK_CUR);

	colorArray = colors.colPointer;

	for(int i = 0; i < colors.size; i ++){
		RGB_to_hex(hex, colorArray[i].rgb);
		snprintf(stringToWrite, 15, "%d:%6s,",colorArray[i].colorNum, hex);
		if( write(fd, stringToWrite, sizeof(char) * strlen(stringToWrite) ) < 0 ){
			perror("Error saving to file");
			show_message_top_left("Error saving colors in file", NULL);
			return -1;
		}
	}
	write(fd, "\n", sizeof(char));
	return lseek(fd, 0, SEEK_CUR);
}

int load_image_from_file(Context *app, char *file_name){
	int fd;
	int i, j, nread, len, filePosition;
	char stringExitMsg[65];
	chtype *bufferPointer;
	size_t size;

	fd = open(file_name, O_RDONLY);
	if (fd < 0){
		show_message_top_left(" Error opening the file ", NULL);
		refresh();
		return -1;
	}
	
	bufferPointer = (chtype*) malloc((app->theDrawWin->cols + 1) * sizeof(chtype));
	memset(bufferPointer, 0, (app->theDrawWin->cols+1) * sizeof(chtype));
	
	len = strlen(file_name);
	if( isCurse(file_name, len) ){

		size = sizeof(chtype);

		filePosition = initialize_colors_from_file(fd, 0, app);
		if(filePosition < 0 ||
				initialize_pairs_from_file(fd, filePosition, app) < 0 ){
			show_message_top_left(" Error initializing colors from file ", NULL);
			free( bufferPointer );
			close(fd);
			return -2;
		}
	}
	else{
		size = sizeof(char);
	}

	for(i = 0; i < app->theDrawWin->lines; i++){
		for(j = 0; j < app->theDrawWin->cols + 1; j++){
			nread = read(fd,&bufferPointer[j], size);
			if ( bufferPointer[j] == '\n' || nread <= 0)
				break;
			if( i == 0 && j == 0 ) clear_Win(app->theDrawWin);
		}
		if (nread >= 0) {
			bufferPointer[j] = 0;
			mvwaddchnstr(app->theDrawWin->ptr, i, 0, bufferPointer, j);
		}
		if (nread <= 0) break;
	}
	
	free( bufferPointer );
	close(fd);

	if (nread >= 0) {
		snprintf(stringExitMsg, 64, " Done loading file: %s ", file_name);
		touchwin(app->theDrawWin->ptr);
	}
	else {
		strncpy(stringExitMsg, "Error reading from file", 31);
	}
	
	app->state->chMask = get_attr_with_color_pair(app->state->chMask, 0);
	wattrset(app->theDrawWin->ptr, app->state->chMask);
	show_message_top_left(stringExitMsg, NULL);
	refresh();
	return 0;
}


int save_drawing_to_file(Context *app, char *file_name){
	int fd;
	chtype *buffer;
	char stringExitMsg[32];
	int filePosition;
	
	buffer = (chtype*) malloc(app->theDrawWin->cols * sizeof(chtype) +1);
	
	fd = open(file_name, O_WRONLY | O_CREAT, 0644);
	if (fd < 0){
		strncpy(stringExitMsg, " Error opening/creating the file ", 32);
		perror(stringExitMsg);
		show_message_top_left(stringExitMsg, NULL);
		refresh();
		free(buffer);
		return 1;
	}

	filePosition = write_colors_in_file(fd, 0, app->custom_colors);
	if( filePosition < 0 )
		return -1;
	else{
		filePosition = write_pairs_in_file(fd, filePosition, app->color_pairs);
		if( filePosition < 0 ) return -1;
	}

	for(int i = 0; i < app->theDrawWin->lines; i++){
		mvwinchstr(app->theDrawWin->ptr, i, 0, buffer);
		write(fd, buffer, sizeof(chtype) * app->theDrawWin->cols);
		*buffer = '\n';
		write(fd, buffer, sizeof(chtype));
	}
	free(buffer);
	close(fd);

	sprintf(stringExitMsg, " Done saving file, size: %lu ",
			app->theDrawWin->cols * app->theDrawWin->lines * sizeof(char) + sizeof(char) * app->theDrawWin->lines );
	show_message_top_left(stringExitMsg, NULL);
	refresh();
	return 0;
}

void show_message_top_left(char* message, int *value){
	mvaddch(0, 0, ACS_ULCORNER);
	mvhline(0, 1, ACS_HLINE, COLS - 2);
	mvaddstr(0, 2, message);
	if( value != NULL ){
		mvprintw(0, strlen(message) + 2 , "%d", *value);
	}
	refresh();
}
