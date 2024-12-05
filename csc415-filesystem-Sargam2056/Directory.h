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
* Description:: Directory struct for File System Project
*
**************************************************************/

#include <time.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <stdint.h>
#define NAME_LENGTH 64

#ifndef _DIRECTORY_H
#define _DIRECTORY_H

typedef struct DIRECTORY {
    // name of Dir and if it is empty => Free blob
    char name[NAME_LENGTH];
    // Size of this Dir entries 
    int size;
    // Blocks allocated to file
    int numBlocksAllocated; // Don't use for directories
    // Index of the started block
    int dir_location;
    // Date when we create this Dir entry
    time_t date_created;
    // Date of last time we opened/accessed/read
    time_t last_accessed;
    // Date of last time the directory was modified
    time_t last_modified;
    // Show that this directory is a directory
    int isDirectory;
} DIR_Entry;


extern DIR_Entry* root;
extern char* bitMap;
extern int g_BlockSize;

// extern int dirEntries;
// extern int dir_blocks;
extern char* cwd;
extern DIR_Entry* currentDirEntry;
#endif