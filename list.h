#ifndef LIST_H
#define LIST_H

#include <stdlib.h>

struct Win;
typedef struct Win Win;

typedef struct WinListElement{
	Win *head;
	struct WinListElement *tail;
} WinListElement; 

typedef struct WinListElement* WinList;

WinList emptyList(void); // PRIMITIVE

int is_empty(WinList list);

Win* head(WinList list);

WinList tail(WinList list);

WinList cons(Win* win, WinList list);

WinList append(Win* win, WinList list);

WinList append_list(WinList dest, WinList list);

void free_list(WinList list);

#endif
