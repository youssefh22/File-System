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

#define FULL_BYTE 255	// Value of a byte with all bits set to 1

// Prototypes for init functions
int initFreeSpace();
int initRoot();

int err;	// For errno

vcb_t* vcb;	// Global pointer to the VCB


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
		// Set vcb->cwd to the root directory
		vcb->cwd = vcb->root;

		// vcb->cwd = malloc(vcb->rootSize * vcb->blockSize);
		// if(vcb->cwd == NULL) {
		// 	err = errno;
		// 	perror("vcb->cwd");
		// 	return err;
		// }
		// // LBAread the root direcoty on disk into vcb->root
		// ret = LBAread(vcb->cwd, vcb->rootSize, vcb->rootAddr);
		// if(ret != vcb->rootSize) {
		// 	err = errno;
		// 	perror("vcb->cwd");
		// 	return err;
		// }
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

		// Calculate mapBlocks, the number of blocks the freeMap needs
		vcb->mapBlocks = (vcb->freeLen + blockSize - 1) / blockSize;

		// Initialize the freeSpace now that the above values have been set
		int ret = initFreeSpace();
		if(ret != 1) {
			printf("ERROR: initFreeSpace()\n");
			return -1;
		}
		

		// The root directory will be located after the freeMap
		vcb->rootAddr = vcb->freeMapAddr + vcb->mapBlocks;

		// Will use 1 block for the root directory for now
		vcb->rootSize = 1;

		// Can now initialize the root directory
		ret = initRoot();
		if(ret != 1) {
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

	return 0;
	}
	
	
void exitFileSystem ()
	{
	printf ("System exiting\n");
	free(vcb->root);
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
	// Allocate space and set values to 0
	uint8_t* bitV = calloc(sizeof(*bitV), vcb->freeLen);
	if(bitV == NULL) {
		err = errno;
		perror("bitV");
		return err;
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

/**
 * @brief Initializes the root directory, sets the '.' and '..' entries and writes
 * the root directory to disk. Assigns the root and cwd pointers in the VCB to
 * the newly created root directory.
 * 
 * @return int
 * 	1 if successful
 * 	-1 if failed
 */
int initRoot() {
	// Allocate memory for direcotry entries
	dirEnt_t* root = malloc(sizeof(dirEnt_t) * 8);

	// Initialize directory entry elements
	for(int i = 0; i < 8; i++) {
		root[i].dateTimeCr = 0;
		root[i].dateTimeMd = 0;
		root[i].location = 0;
		root[i].size = 0;
		strncpy(root[i].name, "", 35);
		root[i].attr = 0;
	}

	// Initialize '.' and '..' entries
	strncpy(root[0].name, ".", 35);
	root[0].attr |= ROOT_MASK;
	root[0].size = 2;

	strncpy(root[1].name, "..", 35);
	root[1].attr |= ROOT_MASK;
	root[1].size = 2;


	// Request free block(s)
	uint64_t location = allocBlocks(1);
	if(location == 0) {
		printf("ERROR: allocBlocks()\n");
		return -1;
	}

	// Set location for '.' and '..'
	root[0].location = location;
	root[1].location = location;

	// Write the root directory to disk
	int ret = LBAwrite(root, 1, location);
	if(ret != 1) {
		err = errno;
		perror("initRoot()");
		return err;
	}

	// Set root and cwd pointers in the VCB to root
	vcb->root = root;
	vcb->cwd = root;

	return 1;
}

/**
 * @brief Sets the given bit on the given byte to 1. Function assumes the bit is
 * known to be free before calling
 * 
 * @param byte
 *  Pointer to the byte in the freeMap that contains the bit to be set
 * @param bit
 *  The bit to be set
 */
void setBit(uint8_t* byte, uint8_t bit) {
	bit = 1 << bit;		// set the bitmask
	*byte |= bit;		// OR byte with the bitmask
}

/**
 * @brief Unsets (sets to 0), the given bit on the given byte. Function assumes
 * the bit is known to be set before calling.
 * 
 * @param byte
 *  Pointer to the in the freeMap that contains the bit to be unset
 * @param bit
 *  The bit to be unset
 */
void unsetBit(uint8_t* byte, uint8_t bit) {
	bit = 1 << bit;		// set the bitmask
	bit ^= 0xFF;		// XOR with 0xFF to invert the bitmask
	*byte &= bit;		// AND byte with the bitmask
}

/**
 * @brief Checks if a given bit on a given byte is set to 0 or 1.
 * 
 * @param byte
 * 	The uint8_t from the freeMap that contains the bit to be checked
 * @param bit
 *  The bit to be checked
 * 
 * @return int
 * 	If 1: the bit's value is 0, indicating it is free
 *  If 0: the bit's value is 1, indicated it is not free
 */
int isBitFree(uint8_t byte, uint8_t bit) {
	bit = 1 << bit;				// set the bitmask
	if((byte & bit) == 0) {		// if byte & bit == 0, the bit is free
		return 1;
	}
	else {
		return 0;
	}
}


/**
 * @brief Allocates contiguous free blocks from the freeMap. If the number
 * requested have been found, those bits will be set to 1 on the freeMap.
 * 
 * @param count
 * 	The number of blocks requested
 * 	
 * @return uint64_t
 * 	If non-zero: The first LBA of the contiguous free blocks
 * 	If 0: Failed, was not able to find requested free blocks
 */
uint64_t allocBlocks(int count) {
	int found = 0; // number of free blocks found
	int index = 0; // index in freeMap we are on
	uint64_t prevFree = 0; // previous bit number to check for contiguous allocation
	uint64_t bitNum = 0; // bit number we are on, which corresponds to LBA

	// Loop while there are more blocks requested and there are more blocks to check
	while(found < count && index < vcb->freeLen) {
		// Check if the freeMap byte has all of its bits set to 1
		if(vcb->freeMap[index] != FULL_BYTE) {
			// If not, loop through its bits
			for(int i = 0; i < 8; i++) {
				// Check if the bit is free
				if(isBitFree(vcb->freeMap[index], i)) {
					if(prevFree == bitNum - 1) {	// If the previous free bit was the one before
						found++;					// this, increment found and set prevFree to
						prevFree = bitNum;			// this bit number.
					}
					else {					// If the previous free bit was not the one before this,
						found = 1;			// then they are not contiguous.  Set found to 1 and
						prevFree = bitNum;	// prevFree to this bit number
					}

				}
				if(found == count) {
					break;		// Exit the loop if we have found enough free blocks
				}
				bitNum++;
			}
		}
		// If the byte was full, increment bitNum by 8
		else {
			bitNum += 8;
		}

		index++;
	}

	// Check if we have found the number of blocks requested
	if(found == count) {
		// Set bitNum to the starting index of blocks to be allocated
		bitNum = prevFree - count + 1;

		// Set allocated bits to 1
		for(int i = 0; i < count; i++) {
			// Get index of the byte we need
			index = (bitNum + i) / 8;
			// Get the bit we need
			uint8_t bit = (bitNum + i) % 8;
			setBit(&vcb->freeMap[index], bit);
		}

		// Write the updated freeMap to disk
		int ret = LBAwrite(vcb->freeMap, vcb->mapBlocks, vcb->freeMapAddr);
		if(ret != vcb->mapBlocks) {
			err = errno;
			perror("allocBlocks");
			return 0;
		}
	}
	// If we did not find enough free blocks, set bitNum to 0 to indicate this
	else {
		bitNum = 0;
	}

	return bitNum;
}