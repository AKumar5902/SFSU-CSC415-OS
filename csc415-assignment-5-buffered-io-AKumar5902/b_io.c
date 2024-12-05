/**************************************************************
 * Class::  CSC-415-02 Fall 2024
 * Name::Anuj Kumar
 * Student ID::923792503
 * GitHub-Name::AKumar5902
 * Project:: Assignment 5 – Buffered I/O read
 *
 * File:: b_io.c
 *
 * Description::
 * This file is where we implement the given function of the
 * header file b_io.h. Such function include opening,
 * reading, and closing files (b_open, b_read, b_close).
 *
 **************************************************************/
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>

#include "b_io.h"
#include "fsLowSmall.h"

#define MAXFCBS 20 // The maximum number of files open at one time

// This structure is all the information needed to maintain an open file
// It contains a pointer to a fileInfo strucutre and any other information
// that you need to maintain your open file.
typedef struct b_fcb
{
	fileInfo *fi; // holds the low level systems file info

	// Add any other needed variables here to track the individual open file
	char *buffer;
	int buffIndex;
	int remainingBuff;
	int blockCount;
	int fileIndex;

} b_fcb;

// static array of file control blocks
b_fcb fcbArray[MAXFCBS];

// Indicates that the file control block array has not been initialized
int startup = 0;

// Method to initialize our file system / file control blocks
// Anything else that needs one time initialization can go in this routine
void b_init()
{
	if (startup)
		return; // already initialized

	// init fcbArray to all free
	for (int i = 0; i < MAXFCBS; i++)
	{
		fcbArray[i].fi = NULL; // indicates a free fcbArray
	}

	startup = 1;
}

// Method to get a free File Control Block FCB element
b_io_fd b_getFCB()
{
	for (int i = 0; i < MAXFCBS; i++)
	{
		if (fcbArray[i].fi == NULL)
		{
			fcbArray[i].fi = (fileInfo *)-2; // used but not assigned
			return i;						 // Not thread safe but okay for this project
		}
	}

	return (-1); // all in use
}

// b_open is called by the "user application" to open a file.  This routine is
// similar to the Linux open function.
// You will create your own file descriptor which is just an integer index into an
// array of file control blocks (fcbArray) that you maintain for each open file.
// For this assignment the flags will be read only and can be ignored.

b_io_fd b_open(char *filename, int flags)
{
	if (startup == 0)
		b_init(); // Initialize our system

	//*** TODO ***//  Write open function to return your file descriptor
	//				  You may want to allocate the buffer here as well
	//				  But make sure every file has its own buffer

	// This is where you are going to want to call GetFileInfo and b_getFCB
	// Get file info
	fileInfo *info = GetFileInfo(filename);
	if (info == NULL)
	{
		// File not found
		return -1;
	}

	// Get a file control block
	b_io_fd fcb = b_getFCB();
	if (fcb == -1)
	{
		// No free FCB available
		return -1;
	}

	// Allocate memory for the buffer

	char *buffer = malloc(B_CHUNK_SIZE);
	if (buffer == NULL)
	{
		// Memory allocation failed
		return -1;
	}

	fcbArray[fcb].fi = info;
	/*print methods for troubleshooting
	printf("file name: %s\n", info->fileName);
	printf("file size: %d\n", info->fileSize);
	printf("file location: %d\n", info->location);
	*/
	fcbArray[fcb].buffer = buffer;

	fcbArray[fcb].buffIndex = 0;
	fcbArray[fcb].remainingBuff = 0;
	fcbArray[fcb].blockCount = 0;
	fcbArray[fcb].fileIndex = 0;

	return fcb;
}

// b_read functions just like its Linux counterpart read.  The user passes in
// the file descriptor (index into fcbArray), a buffer where thay want you to
// place the data, and a count of how many bytes they want from the file.
// The return value is the number of bytes you have copied into their buffer.
// The return value can never be greater then the requested count, but it can
// be less only when you have run out of bytes to read.  i.e. End of File
int b_read(b_io_fd fd, char *buffer, int count)
{
	//*** TODO ***//
	// Write buffered read function to return the data and # bytes read
	// You must use LBAread and you must buffer the data in B_CHUNK_SIZE byte chunks.

	if (startup == 0)
		b_init(); // Initialize our system

	// check that fd is between 0 and (MAXFCBS-1)
	if ((fd < 0) || (fd >= MAXFCBS))
	{
		return (-1); // invalid file descriptor
	}

	// and check that the specified FCB is actually in use
	if (fcbArray[fd].fi == NULL) // File not open for this descriptor
	{
		return -1;
	}

	// Your Read code here - the only function you call to get data is LBAread.
	// Track which byte in the buffer you are at, and which block in the file

	// bytes they want from the file is 0
	if (count <= 0)
	{
		return count;
	}

	// if bytes they want are bigger than fileSize
	// we switch count to fileSize have to account for where we are
	//  at in the file too
	if (count > fcbArray[fd].fi->fileSize - fcbArray[fd].fileIndex)
	{
		count = fcbArray[fd].fi->fileSize - fcbArray[fd].fileIndex;
	}

	int bytesRead = 0;

	while (bytesRead < count)
	{
		// If more bytes remain in the buffer, copy directly
		// no need to call LBAread because we already have bytes
		// that can be buffered into ours
		if (fcbArray[fd].remainingBuff > 0)
		{
			int bytesToCopy = fcbArray[fd].remainingBuff;
			if (bytesToCopy > count - bytesRead)
			{
				bytesToCopy = count - bytesRead;
			}
			memcpy(buffer + bytesRead, fcbArray[fd].buffer + fcbArray[fd].buffIndex, bytesToCopy);
			bytesRead += bytesToCopy;
			fcbArray[fd].buffIndex += bytesToCopy;
			fcbArray[fd].remainingBuff -= bytesToCopy;
		}

		// If we have read enough bytes that was asked for,
		// exit the loop
		if (bytesRead >= count)
		{
			break;
		}

		// Calculate how much more to read if needed
		int remainingToRead = count - bytesRead;

		// If there's enough room, read full chunks directly into the buffer
		// need to call LBAread since we need new data
		if (remainingToRead >= B_CHUNK_SIZE)
		{
			int lbaCount = remainingToRead / B_CHUNK_SIZE;
			uint64_t blocksRead =
				LBAread(buffer + bytesRead, lbaCount, fcbArray[fd].fi->location + fcbArray[fd].blockCount);
			fcbArray[fd].blockCount += blocksRead;
			bytesRead += blocksRead * B_CHUNK_SIZE;
		}

		// If we still haven't reached amount user wants copied read one more
		// block since the previous statements gets in block size anyways
		// this will copy when there is less than 512 
		if (bytesRead < count)
		{
			uint64_t blocksRead =
				LBAread(fcbArray[fd].buffer, 1, fcbArray[fd].fi->location + fcbArray[fd].blockCount);
			fcbArray[fd].blockCount += blocksRead;

			// Calculate how many bytes to copy from the buffer then copy
			int bytesToCopy = count - bytesRead;
			memcpy(buffer + bytesRead, fcbArray[fd].buffer, bytesToCopy);
			bytesRead += bytesToCopy;

			// Update buffer state
			fcbArray[fd].buffIndex = bytesToCopy;
			fcbArray[fd].remainingBuff = B_CHUNK_SIZE - bytesToCopy;
		}
	}

	// Update the file index and return the total bytes read
	fcbArray[fd].fileIndex += bytesRead;
	return bytesRead;
}
// b_close frees and allocated memory and places the file control block back
// into the unused pool of file control blocks.
int b_close(b_io_fd fd)
{
	//*** TODO ***//  Release any resources
	if (fd < 0 || fd >= MAXFCBS)
	{
		return (-1);
	}

	if (fcbArray[fd].fi == NULL)
	{
		return -1;
	}

	free(fcbArray[fd].buffer);
	fcbArray[fd].buffer == NULL;
	//printf("File Closed\n");

	return 0;
}
