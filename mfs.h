/**************************************************************
* Class:  CSC-415
* Name: Professor Bierman
* Student ID: N/A
* Project: Basic File System
*
* File: mfs.h
*
* Description: 
*	This is the file system interface.
*	This is the interface needed by the driver to interact with
*	your filesystem.
*
**************************************************************/
#ifndef _MFS_H
#define _MFS_H
#include <sys/types.h>
#include <unistd.h>
#include <time.h>

#include "b_io.h"

#include <dirent.h>
#define FT_REGFILE	DT_REG
#define FT_DIRECTORY DT_DIR
#define FT_LINK	DT_LNK

#define OUR_SIG	0x796F616C6A6F616E

#ifndef uint64_t
typedef u_int64_t uint64_t;
#endif
#ifndef uint32_t
typedef u_int32_t uint32_t;
#endif
#ifndef uint16_t
typedef u_int16_t uint16_t;
#endif
#ifndef uint8_t
typedef u_int8_t uint8_t;
#endif

#define DIR_BLOCKS 1
#define ENTS_PER_DIR 8

// This structure is returned by fs_readdir to provide the caller with information
// about each file as it iterates through a directory
struct fs_diriteminfo
	{
    unsigned short d_reclen;    /* length of this record */
    unsigned char fileType;    
    char d_name[256]; 			/* filename max filename is 255 characters */
	};

// This is a private structure used only by fs_opendir, fs_readdir, and fs_closedir
// Think of this like a file descriptor but for a directory - one can only read
// from a directory.  This structure helps you (the file system) keep track of
// which directory entry you are currently processing so that everytime the caller
// calls the function readdir, you give the next entry in the directory
typedef struct
	{
	/*****TO DO:  Fill in this structure with what your open/read directory needs  *****/
	unsigned short  d_reclen;		/*length of this record */
	unsigned short	dirEntryPosition;	/*which directory entry position, like file pos */
	uint64_t	directoryStartLocation;		/*Starting LBA of directory */
	} fdDir;

// Key directory functions
int fs_mkdir(const char *pathname, mode_t mode);
int fs_rmdir(const char *pathname);

// Directory iteration functions
fdDir * fs_opendir(const char *name);
struct fs_diriteminfo *fs_readdir(fdDir *dirp);
int fs_closedir(fdDir *dirp);

// Misc directory functions
char * fs_getcwd(char *buf, size_t size);
int fs_setcwd(char *buf);   //linux chdir
int fs_isFile(char * path);	//return 1 if file, 0 otherwise
int fs_isDir(char * path);		//return 1 if directory, 0 otherwise
int fs_delete(char* filename);	//removes a file


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



/* Start of our additions to this file */

typedef struct {	// Directory Entry Structure
	time_t 		dateTimeCr;		// Date and Time the file was created
	time_t 		dateTimeMd;		// Date and Time the file was last modified
	uint64_t 	location;		// LBA of this file's first block on disk
	uint32_t 	size;			// If a directory, number of used entries, otherwise file size
	char 		name[35];		// Name and extension (if applicable)
	uint8_t 	attr;			// Attributes
} dirEnt_t;

// Attribute Masks for Directory Entries
#define ROOT_MASK  0b00001111	// This dirEnt is the root directory
#define DIR_MASK 0b00000001	// This dirEnt is a directory

typedef struct {	// VCB structure
	uint64_t 	blockSize;		// Number of bytes per block
	uint64_t 	numBlocks;		// Number of blocks on this volume
	uint64_t 	freeMapAddr;	// LBA of free-space bitmap
	uint64_t 	freeLen;		// Number of bytes in freeMap
	uint32_t 	mapBlocks;		// Number of blocks the free-space map uses
	uint64_t 	rootAddr;		// LBA of the root directory
	uint32_t 	rootSize;		// Number of blocks the root directory occupies
	uint64_t 	ourSig;			// Signature for our filesystem
	int			numEnts;		// Number of directory entries per directory

	/* These are pointers to be used during operation and must be initialized on
	 each run. The values stored on disk will not be valid */
	uint8_t* 	freeMap;		// Pointer to the freeMap
	dirEnt_t* 	root;			// Pointer to the root directory
	dirEnt_t* 	cwd;			/* Pointer to the current working directory, set
								   to the root directory on startup */
	char** 		cwdName;		// Pointer to the absolute path name
} vcb_t;


struct pathInfo {
	enum lastElem { DNE, File, Dir }; 	/* Status of last element in the path 
										 * DNE - Does Not Exist
										 * File - Found and is a file
										 * Dir - Found and is a directory */
	dirEnt_t* parent; // Pointer to the parent of the last element
	uint64_t parentLoc;	// Parent's LBA
	int index; // Index of the last elem in its parent, if it exists
};

// Free Space functions

// Set bit in byte to 1
void setBit(uint8_t* byte, uint8_t bit);

// Set bit in byte to 0
void unsetBit(uint8_t* byte, uint8_t bit);

// Return 1 if bit is 0, 0 if bit is 1
int isBitFree(uint8_t byte, uint8_t bit);

// Return first LBA of contiguous free blocks, 0 if failed
uint64_t allocBlocks(int count);
#endif

