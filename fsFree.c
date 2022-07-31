#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>

#include "fsLow.h"
#include "mfs.h"


//#define FULL_BYTE 255	// Value of a byte with all bits set to 1

extern vcb_t* vcb;
extern int err;

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