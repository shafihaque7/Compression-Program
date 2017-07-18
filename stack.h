// interface for stacks
// Code provided by Prof. Darrell Long

# ifndef _STACK_H
# define _STACK_H
# include <stdint.h>
# include <stdbool.h>
# include <stdlib.h>
# include "huffman.h"

typedef treeNode *itemAddress;
typedef struct stack
{
	uint32_t size;  // How big?
	uint32_t top; // Where's the top?
	itemAddress *entries;  // Array of items to hold it (via calloc)
} stack;

stack *newStack();  // Constructor
void delStack(stack *);  // Destructor

itemAddress pop(stack *);  // Returns the top item
void push(stack *,  itemAddress);  // Adds an item to the top  

bool empty(stack *);  // Is it empty?
bool full(stack *);  // Is it full
# endif
