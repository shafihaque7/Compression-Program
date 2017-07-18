// Code for decoding a file

# include "stack.h"
# include "queue.h"
# include "huffman.h"
# include "code.h"
# include "bv.h"

# include <stdlib.h>
# include <stdio.h>
# include <stdint.h>
# include <errno.h>
# include <unistd.h>
# include <sys/types.h>
# include <sys/stat.h>
# include <fcntl.h>
# include <getopt.h> // to get commands in

/*
 * This program decodes a file, which is the act of reverting a compressed file (small size) to match the original exactly.
 * The compressed source file is sFile and uncompressed original as oFile.
 *
 * @return ZERO
 */

int main(int argc, char **argv)
{
	/*
	* Open the sFile, the compressed source file
	*/

	uint32_t magicNumber = 0xdeadd00d;  // The number that shows whether or not it's been encoded by us
	char const *program = argv[0];
	int bufferSize = 10240;  // Size of reading buffer; NOTE THAT THIS MUST BE AT LEAST 767.
        int fd;  // File Descriptor
	int outputfd;  // File Descriptor for the output file
	char *inputFile = NULL; // This is NOT OPTIONAL and should be provided when called
	char *outputFile = NULL;  // This is NOT OPTIONAL, and should be provided when called
	bool verbose = false; // false by default
	bool printHuffman = false;
	uint64_t outputSize = 0;
	uint16_t treeSize = 0;

	int argument;
        while ((argument=getopt(argc, argv, "vi:o:pf")) != -1)  // Getopt function parses command line arguments until no more (-1) PROVIDED CODE
        {
                switch(argument)
                {
                        case 'v':  // v is chosen; turn on verbose mode, which prints info regarding encoding/deconding process
                        {
                                verbose = true;
                                break;
                        }
                        case 'i':  // i is chosen; specify the inpute file
                        {
                                inputFile = optarg;
                                break;
                        }
                        case 'o':  // o is chosen; specify the output file
                        {
                                outputFile = optarg;
                                break;
                        }
			case 'p':  // p is chosen; print the tree
			{
				printHuffman = true;
				break;
			}
			default:
			{
				perror(program);
				exit(errno);
			}
                }
        }

	if(access(outputFile,F_OK)!=-1)  // Check if outputFile exists. if It does, kill the program
	{
		perror(program);
		exit(errno);
	}

        if(inputFile == NULL)  // If reading from standard input, set descriptor accordingly
        {
                fd = STDIN_FILENO;
        }
        else  // Otherwise open file with flags
        {
                fd = open(inputFile, O_RDONLY);
                if (fd == -1) // If the file cannot be opened, stop program and print an error message
                {
                        perror(program);
                        exit(errno);
                }
        }

	/*
	* 1. Read in the first 4 bytes of the sFile; if it doesn't match 0xdeadd00d, is invalid; display error and quit.
	*/

	int *buf = (int *)calloc(bufferSize,sizeof(int));  // Reading/writing buffer of the buffer size
	ssize_t bytesRead;  // Bytes Read in one given cycle
	bytesRead = read(fd, buf, 4); // Read the file
	if(bytesRead == -1)  // If the file cannot be read, stop program and print error message
	{
		perror(program);
		exit(errno);
	}
	uint32_t gottenNumber=0;
	for(uint32_t i = 0;i<bytesRead;i++)
	{
		gottenNumber = gottenNumber+(uint32_t)buf[i];  // Get the gotten number from the buffer
	}
	if(gottenNumber!=magicNumber)  // Check if the file is in fact encoded by our program
	{
		perror(program);
		exit(errno);
	}

	/*
	* 2. Read next 8 bytes of sFile, the exact size of oFile, which should be exactly the same as original.
	* Note: Use the size for debugging during entire phase if needed.
	*/

	bytesRead = read(fd, buf, 8); // Read the file
        if(bytesRead == -1)  // If the file cannot be read, stop program and print error message
        {
                perror(program);
                exit(errno);
        }
	for(int i = 0;i<bytesRead;i++)
	{
		outputSize = outputSize + buf[i];  // Get the size from the buffer
	}

	/*
	* 3. Next 2 bytes of the sFile is the treeSize.
	* Allocate an array (savedTree) of uint8_t's of length treeSize, and read in sFile for treeSize bytes into savedTree.
	* This should hold all binary info of the tree into the array, which will be used to reconstruct huffman tree from.
	*/

	uint16_t treeSizeRead[1];
	bytesRead = read(fd, treeSizeRead, 2); // Read the file
	if(bytesRead == -1)  // If the file cannot be read, stop program and print error message
	{
        	perror(program);
        	exit(errno);
    	}
	treeSize = treeSizeRead[0];

	/*
	* 4. Reconstruct the tree using loadTree. (Use a stack; it's written in POST-ORDER, so Leaf, Leaf, parent, etc)
	*/

	bytesRead = 0;  // Reset Bytes read
	uint8_t treeBytes[treeSize];
	uint8_t *savedTree = (uint8_t *)calloc(treeSize,sizeof(uint8_t));  // Create an array of uint8_ts (i.e. LaLbI etc)


	bytesRead = read(fd, treeBytes, treeSize); // Read the file
        if(bytesRead == -1)  // If the file cannot be read, stop program and print error message
        {
                perror(program);
                exit(errno);
        }
        for(uint16_t i = 0;i<treeSize;i++)
        {
                savedTree[i] = treeBytes[i];  // Get the savedTree from the buffer
        }  // Read the file until we've cleared the Tree size.

	treeNode *huffmanTree = loadTree(savedTree,treeSize);  // Create a new variable that points to our newly decompressed huffman tree.
	free(savedTree);  // Free the saved Tree

	if(printHuffman)
	{
		printTree(huffmanTree,1);
	}

	/*
	* 5a. Read from sFile in bits.
	* Create a bit vector of length outputSize*8 containing all of te words in the file in binary form according to the tree
	*/

	uint64_t bitVSize = outputSize*256;  // Note that 256 is max # of leaves, so we create a bit vector for worst case scenario
	bitV *contentVector = newVec(bitVSize);  // Create the bit vector so it has enough space for outputSize bytes.

	uint64_t totalBytes = 0;  // Complete total number of bytes added, to be used for allocating accurately to the bit vector
        do
        {
		uint8_t *symbolsRead = (uint8_t *)calloc(bufferSize,sizeof(uint8_t));
                bytesRead = read(fd, symbolsRead, bufferSize); // Read the file
                if(bytesRead == -1)  // If the file cannot be read, stop program and print error message
                {
                        perror(program);
                        exit(errno);
                }
		// Read all the bits in the bytesREad into bitvector Until end of buffer OR end of output size.

		for(uint64_t byteIndex = 0; (byteIndex<(uint64_t)bytesRead);byteIndex++)
		{
			// To read each bit of each byte
			// Note that if given a 0101 as code, the code stored should be 0000 1010, NOT 0101 0000.
			for(uint8_t bitIndex=0;bitIndex<8;bitIndex++)
			{
				uint8_t bitStream = (symbolsRead[byteIndex] & (0x1 << (bitIndex % 8))) >> (bitIndex % 8); // Translate the byte into a bit
				if(bitStream == 1)  // If it's a 1, set it in the bit vector
				{
					setBit(contentVector,((totalBytes*8)+bitIndex));  // Find the spot and set it
				}  // Otherwise, let it stay 0
			}
			totalBytes++;
		}
		free(symbolsRead);  // Clear the temp reading buffer
        }
        while(bytesRead == bufferSize);  // Keep reading and outputting sections of the text until the total is read

	uint8_t *symbolBuffer = (uint8_t *)malloc(outputSize*sizeof(uint8_t));  // Create an array to hold all of the symbols of the original file.

	/*
	* 5b. Read through the bit vector, and output to the output file the correct symbols by traversing the tree.
	*/

	treeNode *currentTree = huffmanTree;
	// Repeat until all bits in the sFile has been exhausted
	uint64_t symbolIndex = 0;
	for(uint64_t bitIndex = 0; symbolIndex < outputSize; bitIndex++)
	{
		int32_t gottenSymbol;  // What we're looking for
		do
		{
			gottenSymbol = stepTree(huffmanTree,&currentTree,valBit(contentVector,bitIndex));
			if(gottenSymbol == -1)  // If we didn't get anything, decrement the index for the next run.
			{
				bitIndex++;
			}
		}
		while(gottenSymbol == -1);  // Keep stepping through the tree until we get something for the symbol
		symbolBuffer[symbolIndex] = gottenSymbol;  // Now that we've gotten a symbol, add it to the buffer..
		symbolIndex++;  // Increment index for next symbol
	}

	outputfd = open(outputFile,O_CREAT|O_WRONLY,0644);  // Open the new file, the file to be written to (oFile)
        if (outputfd == -1) // If the file cannot be opened, stop program and print an error message
        {
        	perror(program);
        	exit(errno);
        }

	ssize_t bytesWritten = 0;
	bytesWritten = write(outputfd,symbolBuffer,symbolIndex);  // Write to the file
	if(bytesWritten == -1)
	{
		perror(program);
		exit(errno);
	}

	// If verbose, print out the stuff needed
	if(verbose)
	{
		printf("Original %lu bits: tree (%u)\n",(outputSize*8),treeSize);
	}

	// Close both files and anything used. Have a nice night.
	delTree(huffmanTree);
	delVec(contentVector);
	free(symbolBuffer);
	free(buf);

	int closed = close(fd); // Close the file descriptor of the file
        if (closed == -1)  // If error occurs, close program.
        {
                perror(program);
                exit(errno);
        }

	int closedOutput = close(outputfd); // Close the file descriptor of the file
        if (closedOutput == -1)  // If error occurs, close program.
        {
                perror(program);
                exit(errno);
        }
	return 0;  // Successful execution.
}
