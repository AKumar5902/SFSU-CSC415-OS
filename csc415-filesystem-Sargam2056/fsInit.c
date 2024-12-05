/**************************************************************
* Class::  CSC-415-02 - Fall 2024
* Name:: Lap Pham, Anuj Kumar, Sargam Bhatta Lama, Chance Vodnoy
* Student IDs:: 923751501, 923792503, 921854021, 922607267
* GitHub-Name:: lap1597, AKumar5902, Sargam2056, esgameco
* Group-Name:: Know Nothing
* Project:: Basic File System
*
* File:: fsInit.c
*
* Description:: Main driver for file system assignment.
*
* This file is where you will start and initialize your system
*
**************************************************************/

#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <stdio.h>
#include <string.h>

#include "fsLow.h"
#include "mfs.h"
#include "Directory.h"
#include "VolumeControlBlock.h"
#include "freeSpaceManagement.h"

// Global Variables
VCBlock* vcb = NULL; // VCB struct
DIR_Entry* root = NULL; // Root directory entry
DIR_Entry* currentDirEntry = NULL; // Current working directory struct
char* cwd; // Current working directory string
char* bitMap; // Char array for bitmap
int bitMapSize = 0; // Size of the bitmap for indexing
int g_BlockSize; // Global block size from initFileSystem arg0

int initFileSystem (uint64_t numberOfBlocks, uint64_t blockSize) {
	printf ("Initializing File System with %ld blocks with a block size of %ld\n", numberOfBlocks, blockSize);

	// Number of bits needed to store the bitMap
	/**
	 * Since we have 19531 blocks and / 8 = 2441.4  and / 512 = 4.7 
	 * so we need a ceil function here to have 5 blocks for freespace
	*/
	g_BlockSize = blockSize;
	bitMapSize = ceil(((double)numberOfBlocks/8)/(double)blockSize); 
	// Actual space for our bitMap
	bitMap = malloc(bitMapSize*blockSize);
	vcb = malloc(blockSize);

	// Using professor steps for milestone1
	// Follow steps in milestone1 - 
	LBAread(vcb, 1, 0);

	// Check the magic number  and init VCB if it hasn't initialized.
	if (vcb->signature_num != SIGNATURE) {
		//a. Initialize the values in your volume control block
		vcb->block_count = numberOfBlocks;
		vcb->block_size = blockSize;
		vcb->signature_num = SIGNATURE;
		vcb->freeSpotIndex = 0;

		//b. Initialize free space
		for (int i = 0; i < bitMapSize + 1; i++){
			setBit(bitMap, vcb->freeSpotIndex);
			vcb->freeSpotIndex++;
		}
		LBAwrite(bitMap, bitMapSize, 1);

		//c. Initialize the root directory 
		root = make_directory(NULL);

		//d. Set the values returned from above in the VCB
		vcb->root_location = root->dir_location;
		
		//e. LBAwrite the VCB to block 0
		LBAwrite(vcb, 1, 0); // Write vcb
		
	}

	/**
	 * d. If it matches you have initialized your volume already – If not you need to
	 * initialize it
	*/
	LBAread(vcb, 1, 0);
	LBAread(bitMap, bitMapSize, 1);
	
	if (root == NULL)
	{
		root = (DIR_Entry*)malloc(sizeof(DIR_Entry) * NUM_DIR_ENTRIES);
		// Number of blocks gets the num blocks from 1 to n, while we use 0 to n-1
		int numBlocks = ((sizeof(DIR_Entry) * NUM_DIR_ENTRIES) + (g_BlockSize-1))/g_BlockSize;
		// printf("initFileSystem getRoot dir=%p numBlocks=%d\n", root, numBlocks);
		// printf("initFileSystem vcb root_location=%d\n", vcb->root_location);
		LBAread( root, numBlocks-1, vcb->root_location );
	}
	currentDirEntry = root;
	if (cwd == NULL)
	{
		cwd = malloc(MAX_FILENAME);
		strcpy(cwd, "/");
	}
	
	// printf("initFileSystem end\n");
	
	return 0;
}

void exitFileSystem() {
	free(root);
	root = NULL;

	free(bitMap);
	bitMap = NULL;

	free(vcb);
	vcb = NULL;

	// Wasn't working with these:

	// free(cwd);
	// cwd = NULL;
	// free(currentDirEntry);
	// currentDirEntry = NULL;

	printf("System exiting...\n");
}