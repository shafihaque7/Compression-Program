// Code for stacks
// Code created by Arthurlot Li

# include "stdio.h"
# include "stack.h"

# define MIN_STACK 4000 // Arbitrary

// Code for creating a new stack (initially null)

stack *newStack()
{
	stack *new = (stack *)calloc(1,sizeof(stack));  // Create space for the stack
	new->size = MIN_STACK;  // Set the size to minimum of stack as defined
	new->top = 0;  // The top index is initially ZERO.
	new->entries = (itemAddress *)calloc(MIN_STACK,sizeof(itemAddress));  // Create enough space for the entries
	return new;
}

// Self-explanitory nukage  (Change with item)

void delStack(stack *stack)
{
		free(stack->entries);
        free(stack);  // Free the bucket
        return;
}

// Returns the top item of the stack. (Change with item)

itemAddress pop(stack *stack)
{
	stack->top--;  // Decrement the top index
	return stack->entries[stack->top];
}

// Adds an item to the top of the stack.(Change with item)

void push(stack *stack, itemAddress newItem)
{
	if(full(stack))  // If the stack is FULL (need to realloc)
	{
		stack->size = (stack->size)*2;  // Multiply the size by two and adjust the size accordingly
		stack->entries = (itemAddress *)realloc((stack->entries),(stack->size)*sizeof(itemAddress));
	}  // We've ensured that there's enough space now
	stack->entries[stack->top] = newItem;  // Place the item into the array of items in the correct spot.
	stack->top++;  // Increment the top of the stack
	return;
}

// Whether the stack pointer points to null

bool empty(stack *stack)
{
	if(stack->size > 0)  // IF there is something in there
	{
		return 0;  // FALSE, IS NOT EMPTY. (Please don't confuse this.)
	}
        return 1;  // TRUE, IT IS EMPTY
}

// Whether the stack is full in memory

bool full(stack *stack)
{
        return (stack->top == stack->size);
}
