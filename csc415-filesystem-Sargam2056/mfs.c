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
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <stdio.h>

#include "fsLow.h"
#include "mfs.h"
#include <math.h>
#include "freeSpaceManagement.h"
#include <string.h>

// Global Variables
// char* cwd = NULL; // Current working directory
// DIR_Entry* currentDirEntry = NULL;

// Prototypes
// char* processCWD(char *pathname);
int checkFile(char* path);
int findInDir(DIR_Entry* parent, char* token);
DIR_Entry* loadDir(DIR_Entry* dir);

void loadFileBlocks(DIR_Entry* newDir, DIR_Entry* dir, int startingBlock, int blocksNeeded);

/*
    Functions for Milestone 2:

    fs_setcwd
    fs_getcwd
    fs_isFile
    fs_isDir
    fs_mkdir
    fs_opendir
    fs_readdir
    fs_closedir 
    fs_stat

    (optional)
    fs_delete
    fs_rmdir
*/

/*
    Problems:

   
*/

int fs_setcwd(char *pathname)
{    
    // TODO: Fix so "free(): invalid pointer" doesn't happen
    printf("Check pathname %s \n",pathname);
    parsePathInfo parseInfo;

    int parseResult = parsePath(pathname, &parseInfo);

    // If parsepath didnt work, cant find directory
    if (parseResult == -1)
    {
        return -1;
    }
    // If index didnt exist, check if it's root, otherwise error
   
    if (parseInfo.index == 0)
    {
        // Set cwd to root if the parsePath dir is root
        if (parseInfo.dir == root) {
            currentDirEntry = root;
            return 0;
        }
        return -1;
    }

 
    DIR_Entry* tempEntry = loadDir(&parseInfo.dir[parseInfo.index]);

    currentDirEntry = tempEntry;  // cuurent = test2

    char* tempString = malloc(FILENAME_MAX);

    cwd = processCWD(pathname); 

    free(tempString);
    tempString =NULL;
   
    // printf("fs_setcwd after process\n");
    
    freeDir(parseInfo.dir);

    return 0;
    
}

char* processCWD(char *pathname)
{
    printf("CHECK PATH NAME in Process %s\n",pathname);
    char* tempPathName = malloc(MAX_FILENAME);
  
    printf("Entering processCWD\n");


    
    if(pathname[0]=='/'){
       //Absoluate path, do nothing.
    }else{
        
        strcpy(tempPathName, cwd);
        
    }
    strcat(tempPathName, pathname);
    strcat(tempPathName, "/");
        

    // Relative path -- parse
    char* tokens[MAX_TOKENS];
    char* table[MAX_TOKENS];
    char* savePtr;
    int tableIndex = 0;
    int tokenCount = 0;
    char* token;
    char* newPath;
    // Parse path
    // printf("tempPathName=%s\n", tempPathName);
    token = strtok_r(tempPathName, "/", &savePtr);
    // printf("TOKEN %s\n",token);
    while (token != NULL)
    {
  
        tokens[tokenCount] = strdup(token);
        token = strtok_r(NULL, "/", &savePtr);
        tokenCount++;
    }

    // Process tokens
    for (int i = 0; i < tokenCount; i++)
    {
   
        // If ., do nothing
        if (strcmp(tokens[i], ".\0") == 0)
        {
            continue;
        }
        // If .., go to previous
        if (strcmp(tokens[i], "..\0") == 0)
        {
            if (tableIndex > 0)
            {
                tableIndex--;
            }
        }
        else
        {
            table[tableIndex] = tokens[i];
            tableIndex++;
        }
    }
 
    newPath = malloc(MAX_FILENAME);
    strcpy(newPath, cwd);
   strcpy(newPath, "/");

    
    for (int i = 0; i < tableIndex; i++)
    {
  
        strcat(newPath, table[i]);
        strcat(newPath, "/");
    }



    // Free tokens
    for (int i = 0; i < tokenCount; i++)
    {
        free(tokens[i]);
        tokens[i] = NULL;
 
    }
 
    return newPath;
}
int parsePath( const char* path, parsePathInfo* ppi) {
  

    if (path == NULL || strlen(path) == 0) {
        return -1;
    }

    DIR_Entry* start = currentDirEntry;

    // Make a copy of the path to avoid modifying the original
    char* pathCopy = malloc(strlen(path) + 1);
    strcpy(pathCopy, path);



    char* savePtr = NULL;
    char* token = strtok_r(pathCopy, "/", &savePtr);
  

    DIR_Entry* parent = start;

    ppi->dir = start;
    ppi->index = 0;

    ppi->path = malloc(MAX_FILENAME);
    if (token != NULL) {
        strcpy(ppi->path, token);
    } else {
        strcpy(ppi->path, path);
    }

    while (token != NULL) {
        int index = findInDir(parent, token);

        if (index == -1) {
            free(pathCopy);
            return -1;
        }

        if (start[index].isDirectory != 1) {
            free(pathCopy);
            ppi->index = index;
            return 1;
        }

        token = strtok_r(NULL, "/", &savePtr);

        ppi->index = index;

        if (token != NULL) {
            freeDir(parent);
            parent = loadDir(&(parent[index]));

            free(ppi->path);
            ppi->path = malloc(MAX_FILENAME);
            strcpy(ppi->path, token);
            parent = ppi->dir;
        }
    }


    ppi->dir = parent;
    printf("CHECK PPI %s\n",ppi->path);

    if (ppi->dir != parent) {
        freeDir(parent);
    }

   

    free(pathCopy);
    return 0;
}



       
char* fs_getcwd(char *pathname, size_t size)
{
    
    
    return cwd;
}


int fs_isFile(char * filename)
{
    int checkResult = checkFile(filename);
    return checkResult;
}

int fs_isDir(char * pathname)
{
    int checkResult = checkFile(pathname);

    // Reverse result except for -1
    if (checkResult == 0)
        return 1;
    else if (checkResult == 1)
        return 0;
    return checkResult;
}

// Helper Function - Checking whether path is a file
int checkFile(char* path)
{
    // If it is a file returns 1, if not a file returns 0, not found is -1
    parsePathInfo parseInfo;
    int parseResult = parsePath(path, &parseInfo);

    if (parseResult == -1){
        freeDir(parseInfo.dir);
        return -1; // Not found -> -1
    }
    // If index is -1, it's root
    if (parseInfo.index == -1) {
        freeDir(parseInfo.dir);
        return 0; // Directory -> 0
    }
    // Only check for whether it's directory after knowing index is not -1
    if (parseInfo.dir->isDirectory) {
        freeDir(parseInfo.dir);
        return 0; // Directory -> 0
    }

    freeDir(parseInfo.dir);
    return 1; // File -> 1
}

int fs_mkdir(const char *pathname, mode_t mode)
{
   

    parsePathInfo ppi;
    int parseRes = parsePath(pathname, &ppi); //-1

    if (parseRes != -1)
        return -1;

    int freeSpaceIndex = findFreeDirEntry(ppi.dir);
 
    // TODO: Fix
    if (freeSpaceIndex == -1)
        return -1;

    DIR_Entry* newEntry = make_directory(ppi.dir);

 
    strcpy(ppi.dir[freeSpaceIndex].name, ppi.path);
    ppi.dir[freeSpaceIndex].size = newEntry->size;
    ppi.dir[freeSpaceIndex].dir_location = newEntry->dir_location;
    ppi.dir[freeSpaceIndex].date_created = newEntry->date_created;
    ppi.dir[freeSpaceIndex].last_accessed = newEntry->last_accessed;
    ppi.dir[freeSpaceIndex].last_modified = newEntry->last_modified;
    ppi.dir[freeSpaceIndex].isDirectory = 1;



    int blocks = ((ppi.dir->size) + (g_BlockSize-1))/g_BlockSize;

    LBAwrite(ppi.dir, blocks , ppi.dir->dir_location);
    // Free newEntry (not used)
    freeDir(newEntry);
    freeDir(ppi.dir);

    return 0;
}

DIR_Entry* make_directory(DIR_Entry* parent)
{
   
    DIR_Entry* dir = (DIR_Entry*)malloc(sizeof(DIR_Entry) * NUM_DIR_ENTRIES);
    int dir_blocks = ((sizeof(DIR_Entry) * NUM_DIR_ENTRIES) + (g_BlockSize-1))/g_BlockSize;
        
    int start_location = allocBlock(dir_blocks, bitMap, vcb, bitMapSize);
    // printf("make_directory after allocBlock\n");

    for (int i = 0; i < NUM_DIR_ENTRIES; i++) {
        dir->name[0] = '\0';
        dir->size = -1;
        dir->dir_location = -1;
        dir->date_created = -1;
        dir->last_accessed = -1;
        dir->last_modified = -1;
        dir->isDirectory = 1;
    }
    time_t timeStamp;
    time(&timeStamp);

    // Current working directory "."
    strcpy(dir[0].name, ".\0");
    dir[0].size = sizeof(DIR_Entry) * NUM_DIR_ENTRIES;
    dir[0].dir_location = start_location;
    dir[0].date_created = timeStamp;
    dir[0].last_accessed = timeStamp;
    dir[0].last_modified = timeStamp;
    dir[0].isDirectory = 1;

    // Parent directory ".."
    strcpy(dir[1].name, "..\0");
    if (parent == NULL)
    {
        dir[1].size = sizeof(DIR_Entry) * NUM_DIR_ENTRIES;
        dir[1].dir_location = start_location;
        dir[1].date_created = timeStamp;
        dir[1].last_accessed = timeStamp;
        dir[1].last_modified = timeStamp;
    }
    else 
    {
        dir[1].size = parent->size;
        dir[1].dir_location = parent->dir_location;
        dir[1].date_created = parent->date_created;
        dir[1].last_accessed = parent->last_accessed;
        dir[1].last_modified = parent->last_modified;
    }
    dir[1].isDirectory = 1;

    // Write directory to file
    LBAwrite(dir, dir_blocks, start_location);

    return dir;
}

fdDir * fs_opendir(const char *pathname)
{
    printf("fs_opendir\n");
    printf("CHECK PATH NAME: %s\n", pathname);
    parsePathInfo parseInfo;
    int parseResult = parsePath(pathname, &parseInfo);
    //we lost / after ParsePath
        
    printf("CHECK PATH AGAIN %s \n",pathname);
    fdDir *fdDir = malloc(sizeof(fdDir));
    if (fdDir == NULL)
        return NULL;
    if (parseInfo.dir == NULL)
    {
        fs_closedir(fdDir);
        return NULL;
    }

    fdDir->directory = loadDir(parseInfo.dir);
    DIR_Entry* t = fdDir->directory;
    int index = 0;
    while (t->name[0] != '\0')
    {
      
        index++;
        t = &fdDir->directory[index];
    }

    if (fdDir->directory == NULL) {
        free(fdDir);  // Free if loadDir failed
        return NULL;
    }

    fdDir->d_reclen = parseInfo.dir->size / sizeof(DIR_Entry);
    fdDir->dirEntryPosition = 0;
    // int numEntries = fdDir->d_reclen / sizeof(DIR_Entry);
    // fdDir->di = malloc(sizeof(struct fs_diriteminfo) * numEntries);
    int numEntries = fdDir->d_reclen / sizeof(DIR_Entry);
    fdDir->di = malloc(sizeof(struct fs_diriteminfo) * numEntries);

    // printf("fs_opendir end\n");

    return fdDir;
}

struct fs_diriteminfo *fs_readdir(fdDir *dirp)
{
    // printf("fs_readdir dirp=%p\n", dirp);
    // printf("fs_readdir dirp->reclen=%d\n",dirp->d_reclen);
    // printf("fs_readdir dirp->dirEntryPosition=%d\n", dirp->dirEntryPosition);

    if (dirp->dirEntryPosition >= dirp->d_reclen)
        return NULL;

    if (dirp->dirEntryPosition == dirp->d_reclen-1 || 
        dirp->directory[dirp->dirEntryPosition].name[0] == '\0')
    {
        return NULL;
    }

    DIR_Entry* currentEntry = &dirp->directory[dirp->dirEntryPosition];
    printf("size of dirp->di[dirp->dirEntryPosition].d_name,%ld\n",sizeof(dirp->di[dirp->dirEntryPosition].d_name));
    printf("size of currentEntry->name%ld\n",sizeof(currentEntry->name));
    
    strcpy(dirp->di[dirp->dirEntryPosition].d_name, currentEntry->name);

    dirp->di[dirp->dirEntryPosition].d_reclen = currentEntry->size;
    dirp->di[dirp->dirEntryPosition].fileType = currentEntry->isDirectory; 
    dirp->dirEntryPosition++;

    return &(dirp->di[dirp->dirEntryPosition-1]);
}

int fs_closedir(fdDir *dirp)
{
    // free(dirp->di);
    // dirp->di=NULL;
    free(dirp);
    dirp=NULL;
    // TODO: actually do something here

    return 0;
}

int fs_stat(const char *path, struct fs_stat *buf){

    DIR_Entry* tempDir;
    parsePathInfo parseInfo;

    char temp[MAX_FILENAME];
    strcpy(temp, path);
    int parseResult = parsePath(temp, &parseInfo);

    if( parseResult == -1 )
    {
        freeDir(parseInfo.dir);
     
        return -1;
    }

    tempDir = &parseInfo.dir[parseInfo.index];

    buf->st_blocks = (tempDir->size + (g_BlockSize-1))/g_BlockSize;
    buf->st_blksize = g_BlockSize;
    buf->st_size = tempDir->size;
    buf->st_createtime = tempDir->date_created;
    buf->st_accesstime = tempDir->last_accessed;
    buf->st_modtime = tempDir->last_modified;


    freeDir(parseInfo.dir);

    return 0;
}


int findFreeDirEntry(DIR_Entry* parent) {
    // Return free spot index if found, -1 if not found
   
    int numEntries = parent->size / sizeof(DIR_Entry);

    for (int i = 2; i < numEntries; i++)
    {
        if (parent[i].name[0]== '\0'){
            return i;
        }
    }

    return -1;
}

int findInDir(DIR_Entry* parent, char* token) {
    // Loop through, (for), n entries, strcmp
   
    if (token == NULL)
        return -1;
    
    int numEntries = parent->size / sizeof(DIR_Entry);
    for (int i = 0; i < numEntries; i++)
    {
        if (strcmp(parent[i].name, token) == 0)
        {

            return i;
        }
    }

    return -1;
}

DIR_Entry* loadDir(DIR_Entry* dir) {

    if (dir == NULL){
        return NULL;
    }
   
    if(dir->isDirectory != 1){
        
        return NULL;
    }

    int blocksNeeded = (dir->size + (g_BlockSize-1))/g_BlockSize;
    int bytesNeeded = blocksNeeded * g_BlockSize;

    DIR_Entry* new = malloc(bytesNeeded);
    loadFileBlocks(new, dir, 0, blocksNeeded);
   

    return new;
}

void freeDir(DIR_Entry* dir) {
   
    if (dir == root || dir == currentDirEntry)
        return;

    free(dir);
    dir = NULL;
}

// Starting blocks needs to be 0
void loadFileBlocks(DIR_Entry* newDir, DIR_Entry* dir, int startingBlock, int blocksNeeded) {
 
    LBAread(newDir, blocksNeeded, dir->dir_location+startingBlock);
}