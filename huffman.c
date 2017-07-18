// Code for Huffman trees
// Note that printTree was provided entirely by Prof. Darrell Long

# include "huffman.h"
# include "stack.h"
# include <stdio.h>
# include <ctype.h>
# include <unistd.h>
# include <inttypes.h>

// New node, with symbols, leaf or not, a count associated with it (Note that a single node is itself a tree.)

treeNode *newNode(uint8_t symbol, bool leaf, uint64_t count)
{
	treeNode *new = (treeNode *)calloc(1,sizeof(treeNode));  // Allocate the memory for the node
	if(leaf)  // Is it supposed to be a leaf?
	{
		new->symbol = symbol;  // If so, set the symbol accordingly.
	}
	else  // Not a leaf. It's a branch.
	{
		new->symbol = '$';  // Set it to $. (Note that means even if there was a symbol provided it's $.)
	}
	new->leaf = leaf;  // Oh yeah, and set the leaf boolean to whatever it is.
	new->count = count;  // Set the count as well.
	new->left = NULL;  // The leftNode is by default NULL. Note that left is supposed to be branches (leaf if end)
	new->right = NULL;  // The rightNode is by default NULL. Note that right is supposed to be leaves? Leafs? idk.
	return new;
}

// Dump the ENTIRE Huffman tree onto file
// This means that leaves are written as "L" folowed by content (i.e. "a"). Internal nodes are "I"
// Use recursion for post-order traversal.
// USED IN ENCODING
void dumpTree(treeNode *tree, int file) // This is using post order traversal to dump the tree.
{
    if (tree->leaf == true) { // If its a leaf.
        write(file, "L", 1); // Writes L
        write(file, &(tree->symbol), 1); // Writes the symbol.
    } else {

        dumpTree(tree->left, file); // Recursively calls the left size of the tree.
        dumpTree(tree->right, file); // Recursively calls the right part of the tree.
        write(file, "I", 1); // Writes I on the file.
    }
    return;
}
// Build a tree from the saved tree
// Means reconstructing a tree out of the L and I sequence made from dumpTree, which SHOULD BE IN POST ORDER
// USED IN DECODING

treeNode *loadTree(uint8_t savedTree[], uint16_t treeBytes)
{
	stack *tempStack = newStack();// Create a new stack used to setup the tree from the ground up
	// While the tree size has not reached treeBytes
	treeNode *root = NULL;
	for(uint16_t byteIndex = 0;byteIndex<treeBytes;byteIndex++)
	{
		uint16_t token = savedTree[byteIndex];  // Read through each byte of savedTree until the end
		if(token == 'L')  // If the array element is L, the next element is a leaf.
		{
			treeNode *newLeaf = newNode(savedTree[byteIndex+1],true,0);  // Create a new node using newNode. COUNT---------------------
			push(tempStack,newLeaf);  //Now push the node to the stack.
			byteIndex++;  // Skip over the next index, which was the leaf symbol.
		}
		else if(token == 'I')  // Else if the array element is I, Interior node; skip it.
		{
			treeNode *rightChild = pop(tempStack);  // Pop once to get the right child of the interior
			treeNode *leftChild = pop(tempStack);  // Pop again to aquire left child
			root = join(leftChild,rightChild);  // Create the interoir node using join
			push(tempStack,root);  // push the interoir node back into the stack.
		}
		else
		{
			printf("WARNING: savedTree token not recognized!\n");  // This should NEVER HAPPEN. If it does, dumptree failed!
		}
	}
	delStack(tempStack);  // Free the stack!
        return root;
}

// Step through a tree following the code
// Returns a -1 if didn't find the leaf node on a given step, otherwise returns the 8-bit symbol.
// USED IN DECODING

int32_t stepTree(treeNode *root, treeNode **tree, uint32_t code)
{
	if(code == 0)  // If it's a 0,
	{
		*tree = (*tree)->left;  // step LEFT.
	}
	else  // Else it's a 1,
	{
		*tree = (*tree)->right;  // step RIGHT.
	}
	if((*tree)->leaf == true)  // Now that we've stepped, Is it a leaf?
	{
		uint32_t result = (uint32_t)((*tree)->symbol);  // If so, return the symbol for that leaf node
		*tree = root;  // and reset state to be back to root.
		return result;
	}
	else  // Else Is it an interior node?
	{
		return -1;  // IN that case, simply return -1, signifying that a leaf node has not yet been recieived
	}
}

// Parse a Huffman tree to build codes; all this goes onto the stack defined by code.h
// the code s refers to the current path to a leaf, which will change as tree is traversed.
// Any time you come accross a leaf, save the state of the leaf to the table position corersponding to the leaf found.
// Note that code table[256] is the histogram values showing the counts.
// Perform a post order traversal of the tree
// USED IN ENCODING

void buildCode(treeNode *tree, code *s, code *table[256]) // We got serious amount of help from the ta for this one.
{
    // If the tree is valid
    uint32_t x; // x should be a 0 or a 1
    if (tree->leaf)
    {
		int i = (int)tree->symbol; // Gets the tree symbol and stores it as a index on i.
		appendCode(table[i],s); // Calls append code which merges the code together.
        popCode(s, &x); // Pops the code.
        return;
    }
    else
    {// If current node is a leaf, the current stack (code s) represents the path to the node, and so is the code for it.

		pushCode(s,0); // Push(0), and folow the left link (so if not 1, it's 01 or more)
        buildCode(tree->left,s,table);
        pushCode(s,1); // After returning from left, pop() the stack and push(1), following the right link.
        buildCode(tree->right, s, table);
        popCode(s, &x);
        return;


    }
}

// Delete a tree. Pretty self-explanitory.
// Uses a post order traversal. Note that this is a RECURSIVE FUNCTION.
// USED IN BOTH

void delTree(treeNode *tree)
{
	if(tree != NULL)  // IF the tree itself isn't NULL
	{
		// If both left and right are NOT null and at least ONE is NOT leaf, not end.
		if((tree->left != NULL && tree->right != NULL)&&(tree->left->leaf != true || tree->right->leaf != true))
		{
			delTree(tree->left);  // Recursively call yourself again going down the leftNode, clearing that.
			delTree(tree->right);  // And Recursively call yourself going down the rightNode.
		}
		else  // Else if you HAVE reached end,
		{
			if(tree->left != NULL)  // IF APPLICABLE (note that both nodes could be NULL)
			{
				delNode(tree->left);  // Delete Left
			}
			if(tree->right != NULL)  // IF APPLICABLE
			{
				delNode(tree->right);// Delete Right
			}
		}
		delNode(tree);  // In either case, you finish with deleting yourself.
	}
        return;
}

// Join two subtrees
// (Note that this could mean joining two seperate nodes or two seperate trees, we don't care.)
// This always ends up to be an interior node, or I node, meaning symbol is nothing and L is false.
// USED IN BOTH

treeNode *join(treeNode *leftTree, treeNode *rightTree)
{
	uint64_t newCount = leftTree->count + rightTree->count;  // Accumulate the counts of both left and right into a new count for the joint
	treeNode *joint = newNode('$', false, newCount);  // Create the new node that the left and right will be bounded to
	joint->left = leftTree;  // Set the new node's left
	joint->right = rightTree;  // Set the new node's right
        return joint;  // Return the baby tree.
}

static inline void spaces(int c) { for (int i = 0; i < c; i += 1) { putchar(' '); } return; }

void printTree(treeNode *t, int depth)
{
    if (t)
    {
        printTree(t->left, depth + 1);

        if (t->leaf)
        {
            if (isprint(t->symbol))
            {
                spaces(4 * depth); printf("'%c' (%lu)\n", t->symbol, t->count);
            }
            else
            {
                spaces(4 * depth); printf("0x%X (%lu)\n", t->symbol, t->count);
            }
        }
        else
        {
            spaces(4 * depth); printf("$ (%lu)\n", t->count);
        }

        printTree(t->right, depth + 1);
    }

    return;
}
void walkTree(treeNode *tree, uint16_t *size){
    if (tree->leaf == true) {
        *size+=1;
    }
    else{
        walkTree(tree->left,size);
        walkTree(tree->right,size);
    }
    return;


}
