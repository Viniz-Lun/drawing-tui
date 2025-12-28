#ifndef COLLECTION_H
#define COLLECTION_H

typedef struct collection_t{
	void* colPointer;
	short sizeOfElement;
	int maxDim;
	int size;
} Collection;

void initialize_collection(Collection *col, unsigned int numOfElements, unsigned long sizeOfElement);

void zero_out_collection_contents(Collection *col);

void add_element_to_collection(Collection* collection, void* element);

void* get_element_at_index(Collection* collection, int index);
#endif
