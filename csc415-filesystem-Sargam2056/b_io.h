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

#ifndef _B_IO_H
#define _B_IO_H
#include <fcntl.h>
#include "mfs.h"
#include "Directory.h"
#include "freeSpaceManagement.h"

#define DEFAULT_NUM_BLOCKS 100
// READ 01, WRITE 10, RW 11
// #define O_RDONLY 1 // 01
// #define O_WRONLY 2 // 10
// #define O_RDWR 3   // 11

typedef struct fileInfo
{
	DIR_Entry* dir;
	int index;
} fileInfo;

typedef int b_io_fd;


b_io_fd b_open (char * filename, int flags);
int b_read (b_io_fd fd, char * buffer, int count);
int b_write (b_io_fd fd, char * buffer, int count);
int b_seek (b_io_fd fd, off_t offset, int whence);
int b_close (b_io_fd fd);

#endif

