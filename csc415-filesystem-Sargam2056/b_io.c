/**************************************************************
* Class::  CSC-415-02 - Fall 2024
* Name:: Lap Pham, Anuj Kumar, Sargam Bhatta Lama, Chance Vodnoy
* Student IDs:: 923751501, 923792503, 921854021, 922607267
* GitHub-Name:: lap1597, AKumar5902, Sargam2056, esgameco
* Group-Name:: Know Nothing
* Project:: Basic File System
*
* File:: Directory.h
*
* Description:: Implementing Volume Control block struct
*
**************************************************************/

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h> // for malloc
#include <string.h> // for memcpy
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "b_io.h"

#define MAXFCBS 20
#define B_CHUNK_SIZE 512

fileInfo* getFile(char* filePath);
fileInfo* newFile(char* filePath);


typedef struct b_fcb
{
	/** TODO add al the information you need in the file control block **/
	char *buf;	// holds the open file buffer
	int index;	// holds the current position in the buffer
	int buflen; // holds how many valid bytes are in the buffer
	int currentBlk;
	int numBlocks;
	int flags;
	fileInfo *fi;

} b_fcb;

b_fcb fcbArray[MAXFCBS];

int startup = 0; // Indicates that this has not been initialized

// Method to initialize our file system
void b_init()
{
	// init fcbArray to all free
	for (int i = 0; i < MAXFCBS; i++)
	{
		fcbArray[i].buf = NULL; // indicates a free fcbArray
		fcbArray[i].fi =NULL;
	}

	startup = 1;
}

// Method to get a free FCB element
b_io_fd b_getFCB()
{
	for (int i = 0; i < MAXFCBS; i++)
	{
		if (fcbArray[i].buf == NULL)
		{
			return i; // Not thread safe (But do not worry about it for this assignment)
		}
	}
	return (-1); // all in use
}

fileInfo* getFile(char* filePath)
{
	fileInfo* fi = NULL;

	parsePathInfo ppi;
	int parseResult = parsePath(filePath, &ppi);
	// Found file -> don't create
	
	if (parseResult != 1) {
		freeDir(ppi.dir);
		ppi.dir = NULL;
		return newFile(filePath);
	}

	fi = malloc(sizeof(fileInfo));
	fi->dir = ppi.dir;
	fi->index = ppi.index;
	

	return fi;
}

fileInfo* newFile(char* filePath)
{
	fileInfo* fi = NULL;

	// Needs to be a path
	if (filePath == NULL)
	{
		return NULL;
	}

	parsePathInfo ppi;
	int parseResult = parsePath(filePath, &ppi);
	
	// Found file -> don't create
	if (parseResult != -1) {
		return NULL;
	}
	fi = malloc(sizeof(fileInfo));

	// Get free spacels
	int freeSpaceIndex = findFreeDirEntry(ppi.dir);
	if (freeSpaceIndex == -1)
        return NULL;

	// Allocate blocks on disk
	int location = allocBlock(DEFAULT_NUM_BLOCKS, bitMap, vcb, bitMapSize);
	
	// Create dir entry on the free entry position
	time_t timeStamp;
    time(&timeStamp);

	strcpy(ppi.dir[freeSpaceIndex].name, ppi.path);
    ppi.dir[freeSpaceIndex].size = 0;
	ppi.dir[freeSpaceIndex].numBlocksAllocated = DEFAULT_NUM_BLOCKS;
    ppi.dir[freeSpaceIndex].dir_location = location;
    ppi.dir[freeSpaceIndex].date_created = timeStamp;
    ppi.dir[freeSpaceIndex].last_accessed = timeStamp;
    ppi.dir[freeSpaceIndex].last_modified = timeStamp;
    ppi.dir[freeSpaceIndex].isDirectory = 0;

	

	fi->dir = ppi.dir;
	fi->index = freeSpaceIndex;

	int numBlocks = ((sizeof(DIR_Entry) * NUM_DIR_ENTRIES) + (g_BlockSize-1))/g_BlockSize;

	LBAwrite(ppi.dir, numBlocks -1, ppi.dir->dir_location);

	return fi;
}

// Interface to open a buffered file
// Modification of interface for this assignment, flags match the Linux flags for open
// O_RDONLY, O_WRONLY, or O_RDWR
b_io_fd b_open(char *filename, int flags)
{
	b_io_fd returnFd;
	fileInfo* fi;

	//*** TODO ***:  Modify to save or set any information needed
	//
	//
	printf("At b_open\n");

	if (startup == 0){
		
		
		b_init(); // Initialize our system
	}
	
	char *buf;
	buf = malloc(B_CHUNK_SIZE);
	fi = getFile(filename);
	returnFd = b_getFCB();

	if (buf == NULL)
	{
		perror("Failed to allocate buffer\n");
		return -1;
	}
	
	// fi is NULL
	if (fi == NULL || !fi->dir)
	{
		printf("File not found or could not be created\n");		
		return -1;
	}

	// Is a directory
	if (fi->dir[fi->index].isDirectory == 1)
	{
		
		return -1;
	}

	if (returnFd < 0) {
    printf("No available file descriptors\n");
    free(buf);
    return -1;
	} 
	// invalid
	if (returnFd < 0)
	{
		free(buf);
	}
	
	// initialize variables
	fcbArray[returnFd].fi = fi;
	fcbArray[returnFd].buf = buf;
	//memset(fcbArray[returnFd].buf, 0, B_CHUNK_SIZE);
	fcbArray[returnFd].index = 0;
	fcbArray[returnFd].buflen = 0;
	fcbArray[returnFd].currentBlk = 0;
	fcbArray[returnFd].flags = flags;
	fcbArray[returnFd].numBlocks = (fi->dir[fi->index].size + B_CHUNK_SIZE - 1) / (B_CHUNK_SIZE);

	return returnFd; // all set
}

// Interface to seek function
int b_seek(b_io_fd fd, off_t offset, int whence)
{
	// whence -> 0 = beginning, 1 = end, -1 = current position
	if (startup == 0){
		b_init(); // Initialize our system
	}
	// check that fd is between 0 and (MAXFCBS-1)
	if ((fd < 0) || (fd >= MAXFCBS))
	{
		return (-1); // invalid file descriptor
	}

	int index = fcbArray[fd].index;
	if (whence == 0)
	{
		index = 0;
	}
	else if (whence == 2)
	{
		index = fcbArray[fd].fi->dir[fcbArray[fd].fi->index].size;
	}

	int newIndex = (index + offset) % g_BlockSize;
	int newBlock = fcbArray[fd].currentBlk + ((index + offset) / g_BlockSize);
	int totalSize = ((newBlock - fcbArray[fd].fi->dir[fcbArray[fd].fi->index].dir_location) * g_BlockSize) + newIndex;
	int maxSize = fcbArray[fd].fi->dir[fcbArray[fd].fi->index].size;
	
	// Can't seek too much...
	if (totalSize > maxSize)
	{
		return -1;
	}

	LBAread(fcbArray[fd].buf, 1, newBlock);

	fcbArray[fd].index = newIndex;
	fcbArray[fd].index = newBlock;

	return fcbArray[fd].index; // Change this
	return 0;
}

// Interface to write function

int b_write(b_io_fd fd, char *buffer, int count)
{
	printf("At b_write\n");
    if (startup == 0){
		printf("Inside b_write to check startUp");
        b_init(); // Initialize our system
	}
    // Validate file descriptor


    if ((fd < 0) || (fd >= MAXFCBS) || (fcbArray[fd].buf == NULL))
        return -1; // Invalid file descriptor

    int bytesWritten = 0;       // Total bytes written
    int bytesToWrite = count;   // Remaining bytes to write
    int bufferSpace;            // Available space in current buffer
    int blockSize = g_BlockSize;

    while (bytesToWrite > 0)
    {
        // Calculate remaining space in the buffer
        bufferSpace = blockSize - fcbArray[fd].index;

        if (bufferSpace == 0) // Buffer is full, write it to disk
        {
            LBAwrite(fcbArray[fd].buf, 1, fcbArray[fd].fi->dir[fcbArray[fd].fi->index].dir_location + fcbArray[fd].currentBlk);
            fcbArray[fd].index = 0;       // Reset buffer index
            fcbArray[fd].currentBlk++;    // Move to the next block
            bufferSpace = blockSize;
        }

        // Determine how much to write in this iteration
        int writeNow = (bytesToWrite < bufferSpace) ? bytesToWrite : bufferSpace;

        // Copy data into the buffer
        memcpy(fcbArray[fd].buf + fcbArray[fd].index, buffer + bytesWritten, writeNow);
	
        // Update metadata
        fcbArray[fd].index += writeNow;
        bytesWritten += writeNow;
        bytesToWrite -= writeNow;

        // Allocate new blocks if necessary
        if (fcbArray[fd].currentBlk >= fcbArray[fd].numBlocks)
        {
            int newBlocks = allocBlock(DEFAULT_NUM_BLOCKS, bitMap, vcb, bitMapSize);
            if (newBlocks == -1)
                return -1; // Disk full
            fcbArray[fd].numBlocks += DEFAULT_NUM_BLOCKS;
        }
    }

    // Write final buffer contents to disk
    if (fcbArray[fd].index > 0)
    {
        LBAwrite(fcbArray[fd].buf, 1, fcbArray[fd].fi->dir[fcbArray[fd].fi->index].dir_location + fcbArray[fd].currentBlk);
    }

    // Update file size
    fcbArray[fd].fi->dir[fcbArray[fd].fi->index].size += bytesWritten;

    return bytesWritten;
}



// Interface to read a buffer

// Filling the callers request is broken into three parts
// Part 1 is what can be filled from the current buffer, which may or may not be enough
// Part 2 is after using what was left in our buffer there is still 1 or more block
//        size chunks needed to fill the callers request.  This represents the number of
//        bytes in multiples of the blocksize.
// Part 3 is a value less than blocksize which is what remains to copy to the callers buffer
//        after fulfilling part 1 and part 2.  This would always be filled from a refill
//        of our buffer.
//  +-------------+------------------------------------------------+--------+
//  |             |                                                |        |
//  | filled from |  filled direct in multiples of the block size  | filled |
//  | existing    |                                                | from   |
//  | buffer      |                                                |refilled|
//  |             |                                                | buffer |
//  |             |                                                |        |
//  | Part1       |  Part 2                                        | Part3  |
//  +-------------+------------------------------------------------+--------+

int b_read(b_io_fd fd, char *buffer, int count)
{
	int bytesRead;				  // for our reads
	int bytesReturned;			  // what we will return
	int part1, part2, part3;	  // holds the three potential copy lengths
	int numberOfBlocksToCopy;	  // holds the number of whole blocks that are needed
	int remainingBytesInMyBuffer; // holds how many bytes are left in my buffer
	printf("At b_read\n");
	
	if (startup == 0){
		printf("Inside b_read to check startUp\n");
		b_init(); // Initialize our system
	}
	// check that fd is between 0 and (MAXFCBS-1)
	if ((fd < 0) || (fd >= MAXFCBS))
	{
		return (-1); // invalid file descriptor
	}
	
	remainingBytesInMyBuffer = fcbArray[fd].buflen - fcbArray[fd].index;

	// Limit count to file length - i.e. handle End-Of-File
	int amountAlreadyDelivered = (fcbArray[fd].currentBlk * B_CHUNK_SIZE) - remainingBytesInMyBuffer;
	if ((count + amountAlreadyDelivered) > fcbArray[fd].fi->dir[fcbArray[fd].fi->index].size)
	{
		count = fcbArray[fd].fi->dir[fcbArray[fd].fi->index].size - amountAlreadyDelivered;

		if (count < 0)
		{
			printf("ERROR: Count: %d - Delivered: %d - CurBlk: %d - Index: %d\n", count,
				   amountAlreadyDelivered, fcbArray[fd].currentBlk, fcbArray[fd].index);
		}
	}
	// Part 1 is The first copy of data which will be from the current buffer
	// It will be the lesser of the requested amount or the number of bytes that remain in the buffer

	if (remainingBytesInMyBuffer >= count)
	{				   // we have enough in buffer
		part1 = count; // completely buffered (the requested amount is smaller than what remains)
		part2 = 0;
		part3 = 0; // Do not need anything from the "next" buffer
	}
	else
	{
		part1 = remainingBytesInMyBuffer; // spanning buffer (or first read)
		// Part 1 is not enough - set part 3 to how much more is needed
		part3 = count - remainingBytesInMyBuffer; // How much more we still need to copy

		// The following calculates how many 512 bytes chunks need to be copied to
		// the callers buffer from the count of what is left to copy
		numberOfBlocksToCopy = part3 / B_CHUNK_SIZE; // This is integer math
		part2 = numberOfBlocksToCopy * B_CHUNK_SIZE;

		// Reduce part 3 by the number of bytes that can be copied in chunks
		// Part 3 at this point must be less than the block size
		part3 = part3 - part2; // This would be equivalent to parts & B_CHUNK_SIZE
	}
	if (part1 > 0) // memcpy part 1
	{
		

		memcpy(buffer, fcbArray[fd].buf + fcbArray[fd].index, part1);
		fcbArray[fd].index = fcbArray[fd].index + part1;
	}
	if (part2 > 0) // blocks to copy direct to callers buffer
	{
		// limit blocks to blocks left
	bytesRead = LBAread (buffer+part1, numberOfBlocksToCopy,
	fcbArray[fd].currentBlk += numberOfBlocksToCopy);
	part2 = bytesRead * B_CHUNK_SIZE; //might be less if we hit the end of the file

	}

	if (part3 > 0) // We need to refill our buffer to copy more bytes to user
	{
		// try to read B_CHUNK_SIZE bytes into our buffer
		bytesRead = LBAread(fcbArray[fd].buf, 1,
							fcbArray[fd].currentBlk + fcbArray[fd].fi->dir[fcbArray[fd].fi->index].dir_location);

		bytesRead = bytesRead * B_CHUNK_SIZE;

		fcbArray[fd].currentBlk += 1;
		// we just did a read into our buffer - reset the offset and buffer length.
		fcbArray[fd].index = 0;
		fcbArray[fd].buflen = bytesRead; // how many bytes are actually in buffer

		if (bytesRead < part3) // not even enough left to satisfy read request from caller
			part3 = bytesRead;
		if (part3 > 0) // memcpy bytesRead
		{
			memcpy(buffer + part1 + part2, fcbArray[fd].buf + fcbArray[fd].index, part3);
			fcbArray[fd].index = fcbArray[fd].index + part3; // adjust index for copied bytes
		}
	}
	bytesReturned = part1 + part2 + part3;

	return bytesReturned;
}

// Interface to Close the file
int b_close(b_io_fd fd){

	if(fd < 0 || fd>= MAXFCBS){
		return -1;
	}
	//free buf
	free(fcbArray[fd].buf);
	fcbArray[fd].buf=NULL;

	// free fi
	free(fcbArray[fd].fi);
	fcbArray[fd].fi=NULL;

	return 0;


}
