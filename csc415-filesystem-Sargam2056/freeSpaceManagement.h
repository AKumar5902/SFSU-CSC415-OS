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
* Description:: Designing function signature for Manage free space.
*
**************************************************************/

#ifndef _freeSpaceManagement_h
#define _freeSpaceManagement_h

#include "VolumeControlBlock.h"
#include "Directory.h"
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>

int allocBlock(int numOfBlocks, char* bitmap, VCBlock* volumeBuffer, int bitMapSize);
void setBit(char* bitmap, int location);
int isTaken(char* bitmap, int location);
int releaseLocation(char* bitmap, int location, int numOfBlocks);

extern int bitMapSize;
#endif