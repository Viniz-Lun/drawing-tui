#include "headers/collection.h"

#include <string.h>
#include <stdlib.h>

void initialize_collection(Collection *col, unsigned int numOfElements, unsigned long sizeOfElement){
	col->colPointer = malloc((numOfElements) * sizeOfElement);
	memset(col->colPointer, 0, numOfElements * sizeOfElement); 
	col->maxDim = numOfElements;
	col->size = 0;
	col->sizeOfElement = sizeOfElement;
}

void zero_out_collection_contents(Collection *col){
	col->size = 0;
	memset(col->colPointer, 0, col->sizeOfElement * col->maxDim);
}

void add_element_to_collection(Collection* collection, void* element){
	if( collection->maxDim <= collection->size || collection->size < 0 ) return;
	memcpy((collection->colPointer + (collection->sizeOfElement * collection->size)), element, collection->sizeOfElement);
	collection->size++;
}

void* get_element_at_index(Collection* collection, int index){
	if( index < 0 || index > collection->maxDim || index > collection->size ){
		return NULL;
	}
	return collection->colPointer + (index * collection->sizeOfElement);
}
