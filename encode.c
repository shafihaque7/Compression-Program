# include <stdio.h>
# include <fcntl.h>
# include <sys/types.h>
# include <sys/stat.h>
# include <unistd.h>
# include <stdint.h>
# include <getopt.h>

# include <sys/mman.h>
# include <stdlib.h>
# include <errno.h>
# include "queue.h"
# include "stack.h"
# include "bv.h"
# include "huffman.h"

int main(int argc, char **argv )
{
    int fd = -1; // This is the input file.
	int outputfd = -1; // This is the output file.
	bool verbose = false; // The verbose is false by default.
	bool treeBool = false; // The tree doesn't print by default.
	int argument; // The argument for getopt.
	while(( argument = getopt( argc, argv, "vi:o:p")) != -1 )
	{
		switch(argument)
		{
			case 'v': // If the verbose is true
				verbose = true;
				break;
			case 'i':
				fd = open( optarg, O_RDONLY ); // Opens the input file.
				break;
			case 'o':
				outputfd = open( optarg, O_RDONLY ); // Opens the output file.
				if( outputfd != -1 ) // Checks if the file already exists.
				{
					printf("The compressed file already exists.");
					return 1;
				}
                open(optarg,O_CREAT,0644); // Opens and creates the file.
				outputfd = open(optarg, O_WRONLY | O_APPEND); // Makes outputfd the output file.
				break;
			case 'p':
				treeBool = true; // If three tree is on.
				break;
		}
	}
    size_t fileSize; // This is used to get the size of the file.
    struct stat sz;
    fstat (fd, &sz); // fstat is used to get the file size.
    fileSize = sz.st_size;
    uint64_t fdSize=fileSize; // Make fd size a uint64_t

    uint64_t *count = calloc(256,sizeof(uint64_t)); // Creates 256 uint64_t, calloc, so it is initialized to zero.

	uint8_t oneByte; // It reads one byte at a time.

	while(read(fd, &oneByte, 1))
	{
        count[oneByte] += 1; // The histogram count is incremented if the bit is on.
	}

    count[0] += 1; // Darrell told us to make the first and 255 position bit as one.
	count[255] += 1;

	queue *theQueue  = newQueue(256); // Creates a queue with 256 size.
	for(int i=0;i<256;i++) // Looks through all of the bits.
	{
		if(count[i]>0) // If there is more than one 1 count for that bit.
		{
			treeNode *temp = newNode((uint8_t)i,true,count[i]); // Creates a new node as a leaf. The count is put on there.
			enqueue(theQueue,temp); // Enqueues the queue.
		}
	}

	treeNode *leftNode; // The left node is declared.
	treeNode *rightNode; // The right node is declared.

	while(queueSize(theQueue)>1) // The ta helped with this one. Checks if the queueSize is greater then 1.
	{
		dequeue(theQueue,&leftNode); //Dequeues the leftNode
		dequeue(theQueue,&rightNode); // Dequeues the right Node.
		treeNode *tempTreenode = join(leftNode,rightNode); // Joins the left and the right Node.
		enqueue(theQueue,tempTreenode); // Puts the node back into the queue.

	}
	code *tempCode = newCode(); // Creates a temperary code structure.
	code *codeTable[256]; // Creates a code table.
	for( int i = 0; i < 256; i++ )
	{
		codeTable[i] = newCode(); // Initializes all the code as a newCode.
	}

    treeNode *root = theQueue->Q[theQueue->tail]; // The root is the tail of the queue.
	buildCode(root,tempCode,codeTable); // The buildCode sends the tempCode and the code table.

	uint32_t magicCode = 0xdeadd00d; // The magic code is provided by Darrell.
	write(outputfd, &magicCode, 4); // It's uint32_t so its 4 bytes.

	write(outputfd, &fdSize, 8); // The file size is 8 bytes.

	uint16_t treeSize = 0; // The tree size sould be the two bytes.
	walkTree(root,&treeSize); // We get the leafs from walking the tree.
	uint16_t leafs = treeSize; // The leafs are the size of the tree.
	treeSize = ((3 * treeSize) - 1); // This equation was given by Darrell.
	write(outputfd, &treeSize, 2); // We output the tree size.

	dumpTree(root, outputfd); // We dump the tree afterward.

    int bufferSize =100;
	code *mergeCode = newCode(); // The merges all of the code together.
	ssize_t bytesRead;
	uint8_t buf[bufferSize];
	uint32_t readBy = 1;
    lseek(fd, 0, SEEK_SET); // Lseek allows it to read from the beginning.
	while (readBy!=0) // The TA helped with this function.
	{
		bytesRead = read( fd, buf, bufferSize ); // Reads to the bufferSize
		for( int i = 0; i < bytesRead; i++ ) // It reads to bytesRead.
		{
			appendCode(mergeCode, codeTable[buf[i]]); // mergeCode stores all of the code.
		}
		if( bytesRead < bufferSize ) // When its less than bufferSize that means it is done.
		{
			readBy = 0; // The boolean value is zero, which means the loop ends.
		}
	}
	write(outputfd,mergeCode->bits,(mergeCode->l)/8 + 1); // It prints out the bit vector.


	if(verbose) // This prints out if the verbose is on.
	{
		printf("Original %llu bits: ",fdSize*8); // Prints out the original bit count
		printf("leaves %i (%i bytes)", leafs, treeSize );
        printf(" encoding %i bits (%f%%).\n", mergeCode->l, ((float)mergeCode->l/(fdSize*8))*100); // Prints the encoded count.
	}
    if(treeBool)
	{
		printTree(root, 1); // If the boolean value for the tree is on that means it prints out the tree.
	}
    close(outputfd); // Closes the outut file.
	close(fd); // Closes the input file.
    delTree(root); // Deletes the tree.
	delQueue( theQueue ); // Deletes the queue.
    for( int i = 0; i < 256; i++ ) // Frees each element of the table.
	{
		free(codeTable[i]->bits); // Frees each of the bits.
		free(codeTable[i]); // Frees the whole table.
	}
	free(mergeCode->bits); // Frees the bit of the mergeCode
    free(mergeCode); // Frees the mergeCode
	free(tempCode->bits); // Frees the tempCode
	free(tempCode);
	free(count); // Frees the count.


}
