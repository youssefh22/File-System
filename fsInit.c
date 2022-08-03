/**************************************************************
* Class:  CSC-415-0# Fall 2021
* Names: 
* Student IDs:
* GitHub Name:
* Group Name:
* Project: Basic File System
*
* File: fsInit.c
*
* Description: Main driver for file system assignment.
*
* This file is where you will start and initialize your system
*
**************************************************************/


#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>

#include "fsLow.h"
#include "mfs.h"

// Prototypes for init functions
int initFreeSpace();
int initRoot();

int err;	// For errno

vcb_t* vcb;	// Global pointer to the VCB
uint8_t* bitV;	// Pointer will be to the bitvector we are using for our free map

/**
 * @brief Initializes our filesystem.
 * Checks if the volume has already been initialized by checking for our signature.
 * If it has, loads the freeMap and root directory into memory.
 * If not, initializes the VCB, free-space map, and root directory, writing these
 * to the disk.
 * 
 * @param numberOfBlocks Number of blocks contained on the volume we are initializing on
 * @param blockSize Size of a block initialize the file system with
 * @return int - On success: 0
 * 				 On failure: errno
 */
int initFileSystem (uint64_t numberOfBlocks, uint64_t blockSize)
	{
	printf ("Initializing File System with %ld blocks with a block size of %ld\n", numberOfBlocks, blockSize);
	int ret = -1;

	// Allocate memory for VCB
	vcb = malloc(blockSize);
	if(vcb == NULL) {
		err = errno;
		perror("vcb");
		return err;
	}

	// Read block 0 into the VCB pointer
	ret = LBAread(vcb, 1, 0);
	if(ret != 1) {
		err = errno;
		perror("vcb");
		return err;
	}

	/* If our signature is present and correct, it is already initialized
	 * Allocate memory for the freeMap, root, and cwd pointers in the VCB and
	 * use LBAread() to store the corresponding data in those pointers
	 */
	if(vcb->ourSig == OUR_SIG) {
		// Allocate for vcb->freeMap
		vcb->freeMap = malloc(vcb->blockSize * vcb->mapBlocks);
		if(vcb->freeMap == NULL) {
			err = errno;
			perror("vcb->freeMap");
			return err;
		}
		// LBAread the free-space map on disk into vcb->freeMap
		ret = LBAread(vcb->freeMap, vcb->mapBlocks, vcb->freeMapAddr);
		if(ret != vcb->mapBlocks) {
			err = errno;
			perror("vcb->freeMap");
			return err;
		}
		// Allocate for vcb->root
		vcb->root = malloc(vcb->rootSize * vcb->blockSize);
		if(vcb->root == NULL) {
			err = errno;
			perror("vcb->root");
			return err;
		}
		// LBAread the root directry on disk into vcb->root
		ret = LBAread(vcb->root, vcb->rootSize, vcb->rootAddr);
		if(ret != vcb->rootSize) {
			err = errno;
			perror("vcb->root");
			return err;
		}
	}
	// If our signature is not present or correct, we must initialize the volume
	else {
		// Initialize vcb values
		vcb->blockSize = blockSize;
		vcb->numBlocks = numberOfBlocks;

		// freeMap will start after the VCB, on block 1
		vcb->freeMapAddr = 1;

		// Using a byte array for freeMap, freeLen will be number of bytes
		vcb->freeLen = (numberOfBlocks + 7) / 8;
//		printf("vcb->freeLen: %lu\n", vcb->freeLen);

		// Calculate mapBlocks, the number of blocks the freeMap needs
		vcb->mapBlocks = (vcb->freeLen + blockSize - 1) / blockSize;
//		printf("vcb->mapBlocks: %u\n", vcb->mapBlocks);

		vcb->dirLen = DIR_BLOCKS * blockSize / sizeof(dirEnt_t);
//		printf("vcb->dirLen: %d\n", vcb->dirLen);

		// Initialize the freeSpace now that the above values have been set
		int ret = initFreeSpace();
		if(ret != 1) {
			printf("ERROR: initFreeSpace()\n");
			return -1;
		}
		

		// The root directory will be located after the freeMap
		vcb->rootAddr = vcb->freeMapAddr + vcb->mapBlocks;
//		printf("vcb->rootAddr: %lu\n", vcb->rootAddr);

		// Will use 1 block for the root directory for now
		vcb->rootSize = DIR_BLOCKS;

		// Can now initialize the root directory
		//ret = initRoot();
		if(createDir(0) == NULL) {
			printf("ERROR: initRoot()\n");
			return -1;
		}

		// Set our signature
		vcb->ourSig = OUR_SIG;
		
		// LBAwrite the VCB to disk
		ret = LBAwrite(vcb, 1, 0);
		if(ret != 1) {
			err = errno;
			perror("Writing vcb");
			return err;
		}
	}
	// Set vcb->cwd to the root directory
	vcb->cwd = malloc(vcb->rootSize * vcb->blockSize);
	if(vcb->cwd == NULL) {
		err = errno;
		perror("vcb->cwd");
		return err;
	}
	// LBAread the root directory on disk into vcb->root
	ret = LBAread(vcb->cwd, vcb->rootSize, vcb->rootAddr);
	if(ret != vcb->rootSize) {
		err = errno;
		perror("vcb->cwd");
		return err;
	}

	vcb->cwdName = malloc(MAX_PATH);
	strncpy(vcb->cwdName, "/", MAX_PATH);
	return 0;
	}
	
	
void exitFileSystem ()
	{
	printf ("System exiting\n");
	free(vcb->root);
	free(vcb->cwd);
	free(vcb->cwdName);
	free(vcb->freeMap);
	free(vcb);
	}


/**
 * @brief Initializes the array of bytes used for the freeMap, writes the freeMap
 * to disk and assigns the freeMap pointer to the freeMap to keep in memory
 * 
 * @return int 
 */
int initFreeSpace() {
	int freeSize = vcb->mapBlocks * vcb->blockSize;
	/* Allocate space and set values to 0
	 * This allocates an even multiple of blockSize and there may be bits
	 * at the end which do not correspond to an LBA.
	 */
	bitV = calloc(freeSize, sizeof(*bitV));
	if(bitV == NULL) {
		err = errno;
		perror("bitV");
		return err;
	}

	// Set out of bounds trailing bits to 1
	// Calculate how many bytes and bits must be set to prevent allocating them
	int voidBits = freeSize * 8 - vcb->numBlocks;
	int voidBytes = voidBits / 8;
	voidBits = voidBits % 8;
	for(int i = 1; i <= voidBytes; i++) {
		bitV[freeSize - i] = FULL_BYTE;
	}
	for(int i = 0; i < voidBits; i++) {
		// Set the index of the bit
		uint8_t bit = 8 - (voidBits - i);
		setBit(&bitV[freeSize - (voidBytes + 1)], i);
	}
	
	// Set bit 0 to 1, marking VCB as used
	setBit(&bitV[0], 0);

	// Set a number of bits equal to mapBlocks to 1, marking the block as in use
	for(int i = 1; i <= vcb->mapBlocks; i++) {
		setBit(&bitV[0], i);
	}

	// Write root to disk
	int ret = LBAwrite(bitV, vcb->mapBlocks, vcb->freeMapAddr);
	if(ret != vcb->mapBlocks) {
		err = errno;
		perror("initFreeSpace");
		return err;
	}

	// Set freeMap pointer to the bit vector
	vcb->freeMap = bitV;

	return 1;
}