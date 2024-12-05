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
* Description:: Implementing functiops from freeSpaceManagement.h
* to interact with File System.
*
**************************************************************/

#include "freeSpaceManagement.h"
#include <stdint.h>
#include "mfs.h"
#include "fsLow.h"

// #define CHARSIZE 8 		// Number for the 8 bits in an unsigned char
// #define TOTAL_FREE_SPACE 2560 	// Free space bytes as denoted by MS1 steps
// #define FREE_SPACE_LIMIT CHARSIZE*TOTAL_FREE_SPACE
// #define FREE_SPACE_DEBUG 0
// int freeSpaceLocation = 0;	// Number that signifies beginning of free space
// int numSpace = 0;

/*
Except the root "." and parent "..", we gonna set the whole 
block is 0 to indicate that they are free to be used
*/
int allocBlock(int numOfBlocks, char *bitmap, VCBlock *volumeBuffer, int bitMapSize){
    int freeSpaceIndex = volumeBuffer->freeSpotIndex;

    for (int i = 0; i < numOfBlocks; i++) {
        setBit(bitmap, volumeBuffer->freeSpotIndex); // set 0 to free space
        volumeBuffer->freeSpotIndex++; 
    }

    LBAwrite(bitmap, bitMapSize, 1);
    LBAwrite(volumeBuffer, 1, 0);
    return freeSpaceIndex;
}

void setBit(char* bitmap, int location) {
    /*
    Each byte is 8 bits when we want to know if a position is in-used or free
    we look at whether it is a 0 or 1 so we are going to store
    either 0's or 1's inside.
    Basically, we set 0 to 8 bits  in a byte to indicate it free
    Free-Space Management - Professor Bierman at 1:00s.

    */

    //gives the index inside of array.
    int i = location / 8; 
    //gets the position inside of the index
    int pos = location % 8; 

    //shift the position of the 0 to the position within index
    //now set the position inside the bitmap
    bitmap[i] |= (1 << pos);
}

int isTaken(char* bitmap, int location)
{
    int i = location / 8;
    int pos = location % 8;
    
    if (bitmap[i] & (1 << (pos))) {
        return 1; // indicate the loc is taken, index =1 "AND" 1  => 1.
    }
    else {
        return 0; // free
    }     
    return -1; // I think happen when check thing we don't have
}

/**
 * Based on class lecture,
 * we interate through each bit of the char - 8 bits
 * and use AND operator to set with whatever value 
 * at index to 0.
*/
// TODO: Don't know whether or not this works completely
int releaseLocation(char* bitmap, int position, int numOfBlocks)
{
    for (int k = position; k < position + numOfBlocks; k++) { 
        int index = k / 8;
        int pos = k % 8;
        
        unsigned int clearBit = 1;

        clearBit = clearBit << pos; // shift clearBit to the position inside index
        clearBit = ~clearBit; //change to 0

        bitmap[index] = bitmap[index] & clearBit; //"AND" to change value to 0;
    }

    LBAwrite(bitmap, 5, 1);
}

