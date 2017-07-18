// Interface for huffman trees
// Code provided by Prof. Darrell Long

# ifndef _HUFFMAN_H
# define _HUFFMAN_H
# include <stdint.h>
# include <stdbool.h>
# include <stdlib.h>
# include <stdio.h> // Used for debug printf functions!
# include <ctype.h>  // Used for print tree
# include "code.h"

# ifndef NIL
# define NIL (void *) 0
# endif

typedef struct DAH treeNode;

struct DAH
{
	uint8_t symbol;
	uint64_t count;
	bool leaf;
	treeNode *left, *right;
};

// New node, with symbols, leaf or not, a count associated with it
treeNode *newNode(uint8_t symbol, bool leaf, uint64_t count);

// Dump a Huffman tree onto file
void dumpTree(treeNode *tree, int file);

// Build a tree from the saved tree
treeNode *loadTree(uint8_t savedTree[], uint16_t treeBytes);

// Step through a tree following the code
int32_t stepTree(treeNode *root, treeNode **tree, uint32_t code);

// Parse a Huffman tree to build codes
void buildCode(treeNode *tree, code *s, code *table[256]);

// Delete a tree
void delTree(treeNode *tree);

static inline void delNode(treeNode *h) { free(h); return; }

static inline int8_t compare(treeNode *l, treeNode *r)
{
	return l->count - r->count; // l<r if negative, 1=r if 0...
}

treeNode *join(treeNode *leftTree, treeNode *rightTree);  // Join two subtrees

void printTree(treeNode *tree,int depth); // Debug function (can be called with -p)
void walkTree(treeNode *tree, uint16_t *size);
# endif
