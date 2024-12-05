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

#include <time.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <stdint.h>

#define LENGTH 128
#define SIGNATURE 11212024

#ifndef VCB_H
#define VCB_H


typedef struct VCB {

    unsigned int signature_num; // Number to identify VCB
    int block_count;            // Total free blocks
    int block_size;             // Size of the block, we will pick default 512 bytes
    int root_location;          // Root directory location
    int freeSpotIndex;          // For bitmap, to find free space

} VCBlock;
extern VCBlock* vcb;
#endif

// TODO: Will be used later
// extern VCBblock* vcb;