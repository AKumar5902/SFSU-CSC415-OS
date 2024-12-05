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


#ifndef _MFS_H
#define _MFS_H
#include <sys/types.h>
#include <unistd.h>
#include <time.h>

#include "b_io.h"
//Add header files here
#include "Directory.h"
#include "fsLow.h"
#include "freeSpaceManagement.h"
#include "VolumeControlBlock.h"

#include <dirent.h>
#define FT_REGFILE	DT_REG
#define FT_DIRECTORY DT_DIR
#define FT_LINK	DT_LNK

#ifndef uint64_t
typedef u_int64_t uint64_t;
#endif
#ifndef uint32_t
typedef u_int32_t uint32_t;
#endif
// #ifndef blksize_t
// typedef u_int32_t blksize_t;
// #endif

#define MAX_FILENAME 256
#define MAX_TOKENS 256
#define NUM_DIR_ENTRIES 56

// This structure is returned by fs_readdir to provide the caller with information
// about each file as it iterates through a directory
struct fs_diriteminfo
	{
    unsigned short d_reclen;    /* length of this record */
    unsigned char fileType;    
    char d_name[MAX_FILENAME]; 			/* filename max filename is 255 characters */
	};

// This is a private structure used only by fs_opendir, fs_readdir, and fs_closedir
// Think of this like a file descriptor but for a directory - one can only read
// from a directory.  This structure helps you (the file system) keep track of
// which directory entry you are currently processing so that everytime the caller
// calls the function readdir, you give the next entry in the directory
typedef struct
	{
	/*****TO DO:  Fill in this structure with what your open/read directory needs  *****/
	int  d_reclen;		/* length of this record */
	int	dirEntryPosition;	/* which directory entry position, like file pos */
	int dir_location;
	DIR_Entry *	directory;			/* Pointer to the loaded directory you want to iterate */
	struct fs_diriteminfo * di;		/* Pointer to the structure you return from read */

	
	} fdDir;

// The resulting struct from parsePath
typedef struct
	{
		DIR_Entry* dir;
		int index;
		char* path;
	} parsePathInfo;

// Key directory functions
int fs_mkdir(const char *pathname, mode_t mode);
DIR_Entry* make_directory(DIR_Entry* parent);
int fs_rmdir(const char *pathname);

// Directory iteration functions
fdDir * fs_opendir(const char *pathname);
struct fs_diriteminfo *fs_readdir(fdDir *dirp);
int fs_closedir(fdDir *dirp);

// Misc directory functions
char * fs_getcwd(char *pathname, size_t size);
int fs_setcwd(char *pathname);   //linux chdir
int fs_isFile(char * filename);	//return 1 if file, 0 otherwise
int fs_isDir(char * pathname);		//return 1 if directory, 0 otherwise
int fs_delete(char* filename);	//removes a file

char* processCWD(char *pathname);
int findFreeDirEntry(DIR_Entry* parent);
// This is the strucutre that is filled in from a call to fs_stat
struct fs_stat
	{
	off_t     st_size;    		/* total size, in bytes */
	blksize_t st_blksize; 		/* blocksize for file system I/O */
	blkcnt_t  st_blocks;  		/* number of 512B blocks allocated */
	time_t    st_accesstime;   	/* time of last access */
	time_t    st_modtime;   	/* time of last modification */
	time_t    st_createtime;   	/* time of last status change */
	
	/* add additional attributes here for your file system */
	};

int fs_stat(const char *path, struct fs_stat *buf);

int parsePath( const char* path, parsePathInfo* ppi);
int recursive(char* path, DIR_Entry* Dir);
int getPath(int location);
char* getFileName(DIR_Entry* Dir);
void freeDir(DIR_Entry* dir);
// extern DIR_Entry* currentDirEntry;
// extern char* cwd;

#endif

