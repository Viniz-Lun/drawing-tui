#include "list.h"
#ifndef NULL
	#define NULL (void*)0
#endif

WinList emptyList() { return NULL; }

int is_empty(WinList list) {
	if (list == NULL) return 1;
	else return 0;
}

Win* head(WinList list) {
	if (is_empty(list)) return NULL;
	else return list->head;
}

WinList tail(WinList list) {
	if (is_empty(list)) return NULL;
	else return list->tail;
}

WinList cons(Win* element, WinList list) {
	WinList t;
	t = (WinList)malloc(sizeof(WinListElement));
	t->head = element;
	t->tail = list;
	return t;
}

WinList append(Win* win, WinList list){
	WinList tempList = list;
	WinList newElement;

	newElement = (WinList) malloc(sizeof(WinListElement));
	newElement->head = win;
	newElement->tail = emptyList();

	if ( ! is_empty(tempList) ){
		while ( ! is_empty(tempList) ){
			tempList = tempList->tail;
		}
		tempList->tail = newElement;
	}
	else {
		list = newElement;
	}

	return list;
}

WinList append_list(WinList dest, WinList src){
	WinList tempList = dest;
	if ( ! is_empty(tempList) ){
		while ( ! is_empty(tempList->tail) ){
			tempList = tempList->tail;
		}
		tempList->tail = dest;
	}
	else{
		tempList = src;
	}
	return tempList;
}

void free_list(WinList list) {
	if (is_empty(list))
		return;
	else {
		free_list(tail(list));
		free(list);
	}
	return;
}




